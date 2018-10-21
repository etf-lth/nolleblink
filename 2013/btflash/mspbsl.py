#!/usr/bin/python
#
# MSPBSL
#
# (c) 2012-2013 Fredrik Ahlberg <fredrik@z80.se>

import sys
import struct
import time
import argparse

import tty
import termios

# For reading Intel-Hex
from binascii import unhexlify

# For the FT232 driver
import pylibftdi

# For nolleblink
from bluetooth.bluez import BluetoothSocket

""" Quick and dirty parser for Intel Hex files """
class IHex:

    def __init__(self):
        self.data = []

    """ Parse the hex file """
    def load(self, filename):
        f = file(filename)

        while True:
            l = f.readline().strip()
            if len(l) < 1:
                break

            if l[0] != ':':
                raise Exception('Unexpected start of line, "%c"' % l[0])

            (length, address, type) = struct.unpack('>BHB', unhexlify(l[1:9]))
            data = unhexlify(l[9:9+length*2])

            self.data.append({'type': type, 'addr': address, 'data': data})

    """ Try and condense consecutive lines of same type into larger blocks. """
    """ This allows us to do more efficient flashing later on. """
    def condense(self):
        data = []
        last = None
        size = 0
        for row in self.data:
            if row['type'] == 0:
                size += len(row['data'])
            if last and last['type'] == row['type'] and row['addr'] == last['addr']+len(last['data']):
                last['data'] += row['data']
                continue
            elif last is not None:
                data.append(last)
            last = row
        if last is not None:
            data.append(last)
        self.data = data
        return size


""" Driver for FT232 based interfaces """
class FT232Driver:

    def __init__(self):
        self.s = None

    """ Perform a hard reset on the target and either enter the BSL or normal execution """
    def reset(self, enter_bsl = False):
        del self.s

        # Create a bitbang device to do the secret handshake
        ctl = pylibftdi.BitBangDevice()

        if enter_bsl:
            # RTS (0x04) is TEST, CTS (0x08) is RST
            ctl.direction = 0x0C    # Drive pins
            ctl.port = 0x00         # Drive all low
            ctl.port = 0x04         # Set TEST
            ctl.port = 0x00         # Clear
            ctl.port = 0x04         # Set TEST
            ctl.port = 0x0C         # "Release" RST
            ctl.port = 0x08         # "Release" TEST
            ctl.direction = 0x00    # Release pins
        else:
            ctl.direction = 0x0C    # Drive pins
            ctl.port = 0x00         # Drive all low
            ctl.port = 0x04         # "Release" RST
            ctl.port = 0x0C         # "Release" TEST
            ctl.direction = 0x00    # Release pins

        del ctl

        # Create a simple serial port device for the BSL.
        # N.B the even parity bit!
        self.s = pylibftdi.Device()
        if enter_bsl:
            # 8 bits, even parity, 1 stop bit
            self.s.ftdi_fn.ftdi_set_line_property(8, 0, 2)
        else:
            self.s.ftdi_fn.ftdi_set_line_property(8, 0, 0)
        self.s.baudrate = 9600
        self.s.flush()

    """ Write data to the BSL on the serial port """
    def write(self, data):
        self.s.write(data)

    """ Perform a blocking read on the serial port from the BSL """
    def read(self, length):
        # TODO: We should really have some timeout here...
        buf = ''
        while len(buf) < length:
            buf = buf + self.s.read(length-len(buf))
            time.sleep(0)
        return buf

""" Driver for Nolleblink 2013 """
class NolleDriver:

    def __init__(self, bdaddr = '00:80:37:14:49:42', port = 3):
        self.s = BluetoothSocket()
        self.s.connect((bdaddr, port))

    """ Perform a hard reset on the target and either enter the BSL or normal execution """
    def reset(self, enter_bsl = False):
        if enter_bsl:
            self.s.send('b')
        else:
            self.s.send('x')
        r = self.s.recv(1)
        if r != 'k':
            raise Exception('Unexpected response')

    """ Write data to the BSL on the serial port """
    def write(self, data):
        while len(data) > 0:
            to_send = min(255, len(data))
            d = data[:to_send]
            data = data[to_send:]

            self.s.send('w'+chr(len(d))+d)
            r = self.s.recv(1)
            if r != 'k':
                raise Exception('Unexpected response')

    """ Perform a blocking read on the serial port from the BSL """
    def read(self, length):
        # TODO: We should really have some timeout here...
        buf = ''
        while len(buf) < length:
            self.s.send('r'+chr(length-len(buf)))
            l = ord(self.s.recv(1))
            if l > 0:
                buf = buf + self.s.recv(l)
            time.sleep(0)
        return buf

    """ Read the calibration block from IRMA """
    def read_calibration(self):
        self.s.send('c?')
        r = self.s.recv(1)
        if r == 'k':
            return self.s.recv(10)
        elif r == 'n':
            return None
        else:
            raise Exception('Unexpected response')

    """ Write or delete the calibration block on IRMA """
    def write_calibration(self, cal):
        if cal is None:
            # Delete calibration
            self.s.send('cx')
        else:
            if len(cal) != 10:
                raise Exception('Wrong length of calibration block')
            self.s.send('c'+cal)
        if self.s.recv(1) != 'k':
            raise Exception('Unexpected response')

""" Protocol driver for 1xx, 2xx and 4xx devices """
class BSL2xx:

    """ Create a protocol instance on the specified driver """
    def __init__(self, driver):
        self._driver = driver
        self._last_addr = 0

    """ Internal helper function to calculate the checksum of a packet """
    def _checksum(self, data):
        return reduce(lambda a, b: a^b, map(ord, data[::2]), 0xff) | (reduce(lambda a, b: a^b, map(ord, data[1:][::2]), 0xff) << 8)

    """ Internal function to send a packet to the BSL """
    def _send(self, cmd, l, a = 0, ll = 0, payload = "", noack = False):
        # Perform the syncronization
        self._driver.write(chr(0x80))
        ack = ord(self._driver.read(1))
        if ack != 0x90:
            raise Exception('Not synced: Got %02x, expected 0x90' % ack)

        # Pack the frame and append the resulting checksum
        packet = struct.pack('<BBBBHH', 0x80, cmd, l, l, a, ll)+payload
        packet += struct.pack('<H', self._checksum(packet))

        # Send
        self._driver.write(packet)

        if noack:
            return

        # Affirmative?
        ack = self._driver.read(1)
        if ord(ack) == 0xa0:
            raise Exception('NAK')
        if ord(ack) != 0x90:
            raise Exception('Operation not acked: Got %02x, expected 0x90' % ord(ack))

    """ Internal function to receive a data packet from the BSL when doing a memory read """
    def _recv(self):
        # Receive and unpack the header
        magic = ord(self._driver.read(1))
        if magic == 0xa0:
            raise Exception('NAK')
        elif magic != 0x80:
            raise Exception('Not HDR: %02x' % magic)

        header = self._driver.read(3)
        (_, l1, l2) = struct.unpack('BBB', header)
        if l1 != l2:
            raise Exception('l1 != l2, %02x != %02' % (l1, l2))

        # Read the payload and the checksum
        payload = self._driver.read(l1)
        (checksum, ) = struct.unpack('<H', self._driver.read(2))

        # Everything okay?
        calcd = self._checksum('\x80' + header + payload)
        if calcd != checksum:
            raise Exception("Invalid checksum %04x != %04x" % (checksum, calcd))

        return payload

    """ Reset and enter the BSL """
    def enter_bsl(self):
        print 'Entering bootloader...',
        self._driver.reset(True) 
        print 'done'

    #def set_baud(self):
    #    print 'Changing baud rate...'
    #    self.send(0x52, '\x06')
    #    self.s.flush()
    #    self.s.baudrate = 115200

    #def get_version(self):
    #    self.send(0x1e, 0x04)
    #    print ' '.join(map(hex, map(ord, self.recv())))

    """ Perform a mass erase """
    def mass_erase(self):
        print 'Performing mass erase...',
        self._send(0x18, 0x04, 0, 0xa506)
        print 'done'

    """ Unlock the BSL using the specified password """
    def unlock(self, password = 32*'\xff'):
        if len(password) != 32:
            raise Exception('Wrong password length %d, expecting 32 bytes' % len(password))

        print 'Unlocking BSL...',
        self._send(0x10, 0x24, 0, 0, password)
        print 'done'

    """ Internal helper function to set the high part of the memory address """
    def set_addr(self, addr):
        #print 'Setting high address to %04x...' % addr,
        self._send(0x21, 0x04, 0, addr)
        #print 'done'

    """ Write a data block to memory """
    def write(self, addr, data):
        if addr >> 16 != self._last_addr:
            self._set_addr(addr >> 16)
            self._last_addr = addr >> 16

        #print 'Writing %d bytes to %04x' % (len(data), addr)
        self._send(0x12, len(data)+4, addr, len(data), data) 

    """ Read a data block from memory """
    def read(self, addr, len):
        if addr >> 16 != self._last_addr:
            self._set_addr(addr >> 16)
            self._last_addr = addr >> 16

        #print 'Reading %d bytes at %04x' % (len, addr)
        self._send(0x14, 0x04, addr, len, noack = True)
        return self._recv()

    """ Identify the target device and BSL version """
    def check_version(self):
        (chipid, ) = struct.unpack('>H', self.read(0xff0, 2))
        (bslmajor, bslminor) = struct.unpack('>BB', self.read(0xffa, 2))
        print 'Chip ID: %04x' % chipid
        print 'BSL Version: %d.%02d' % (bslmajor, bslminor)

    #def run(self, addr):
    #    print 'Set PC to %04x' % addr
    #    self.send(0x17, struct.pack('<HB', addr & 0xffff, addr >> 16), noack = True) 

    """ Reset the target and let the application run """
    def reset(self):
        print 'Hard reset'
        self._driver.reset(False)

    def read_calibration(self):
        return self.read(0x10f6, 10)

""" Protocol driver for 5xx and 6xx devices """
class BSL5xx:

    """ Create a protocol instance on the specified driver """
    def __init__(self, driver):
        self._driver = driver

    """ Internal helper function to calculate the checksum of a packet """
    def _checksum(self, data):
        crc = 0xffff

        for byte in data:
            x = ((crc >> 8) ^ (byte & 0xff)) & 0xff
            x ^= (x >> 4)
            crc = ((crc << 8) ^ (x << 12) ^(x << 5) ^ x) & 0xffff

        return crc

    """ Internal function to send a packet to the BSL """
    def _send(self, cmd, payload = "", noack = False):
        core = chr(cmd)+payload
        packet = struct.pack('<BH', 0x80, len(core))+core+struct.pack('<H', self._checksum(map(ord, core)))
        self._driver.write(packet)

        if noack:
            return

        ack = self._driver.read(1)
        if ord(ack) != 0x00:
            raise Exception('Not ack: %02x' % ord(ack))

    """ Internal function to receive a data packet from the BSL when doing a memory read """
    def _recv(self):
        (magic, length) = struct.unpack('<BH', self._driver.read(3))
        payload = self._driver.read(length)
        (checksum, ) = struct.unpack('<H', self._driver.read(2))

        calcd = self._checksum(map(ord, payload))
        if calcd != checksum:
            raise Exception("Invalid checksum %04x != %04x" % (checksum, calcd))

        if len(payload) > 0 and ord(payload[0]) == 0x3b:
            msgs = ("Success", "Flash write check failed", "Flash Fail bit set",
                    "Voltage Change During Program", "BSL Locked", "BSL Password failed",
                    "Unknown command", "Packet buffer overrun")
            if ord(payload[1]) > 0:
                print 'BSL Message: %02x %s' % (ord(payload[1]), msgs[ord(payload[1])])

        return payload

    """ Reset and enter the BSL """
    def enter_bsl(self):
        print 'Entering bootloader...',
        self._driver.reset(True)
        print 'done'

    #""" Change baud rate """
    #def set_baud(self):
    #    print 'Changing baud rate...'
    #    self._send(0x52, '\x06')
    #    time.sleep(0.1)
    #    self.s.baudrate = 115200

    """ Perform a mass erase """
    def mass_erase(self):
        print 'Performing mass erase...',
        self._send(0x15)
        self._recv()
        print 'done'

    """ Unlock the BSL using the specified password """
    def unlock(self, password = 32*'\xff'):
        if len(password) != 32:
            raise Exception('Wrong password length %d, expecting 32 bytes' % len(password))

        print 'Unlocking BSL...',
        self._send(0x11, password)
        self._recv()
        print 'done'

    """ Write a data block to memory """
    def write(self, addr, data):
        #print 'Writing %d bytes to %04x' % (len(data), addr)
        self._send(0x10, struct.pack('<HB', addr & 0xffff, addr >> 16)+data)
        self._recv()

    """ Read a data block from memory """
    #def read(self, addr, len):
#        #print 'Reading %d bytes at %04x' % (len, addr)
    #    self._send(0x14, 0x04, addr, len, noack = True)
    #    return self._recv()

    """ Identify the target device and BSL version """
    def check_version(self):
        self._send(0x19)
        print 'BSL Version: ' + ' '.join(map(hex, map(ord, self._recv())))

    #def run(self, addr):
    #    print 'Set PC to %04x' % addr
    #    self.send(0x17, struct.pack('<HB', addr & 0xffff, addr >> 16), noack = True) 

    """ Reset the target and let the application run """
    def reset(self):
        print 'Hard reset'
        self._driver.reset(False)

def write_progress(written, size):
    width = 32
    sys.stdout.write('\r[');
    dots = width*written/size
    for i in xrange(width):
        if i < dots:
            sys.stdout.write('=')
        else:
            sys.stdout.write(' ')
    sys.stdout.write('] %3d %%' % (100*written/size))
    sys.stdout.flush()

#def terminal(s):
#    old_settings = termios.tcgetattr(sys.stdin)
#    try:
#        tty.setcbreak(sys.stdin.fileno())
#        while True:
#            (inp, _, _) = select.select([sys.stdin, s], [], [])
#            if s in inp:
#                d = s.read(1)
#                sys.stdout.write(d)
#                sys.stdout.flush()
#            if sys.stdin in inp:
#                d = sys.stdin.read(1)
#                s.write(sd)
#    finally:
#        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)

def terminal(driver):
    driver.s.baudrate = 115200
    while True:
        c = driver.read(1)
        sys.stdout.write(c)
        sys.stdout.flush()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = 'MSP430 Bootstrap Loader (c) 2012-2013, Fredrik Ahlberg', prog = 'mspbsl')

    parser.add_argument(
            '--driver', '-d',
            help = 'Hardware interface to use: ft232 (default), nolle',
            default = 'ft232')

    parser.add_argument(
            '--protocol', '-p',
            help = 'BSL Protocol version: 2 (default) or 5',
            default = '2')

    parser.add_argument(
            '--password', '-P',
            help = 'BSL password as 32 hex bytes. Default: Mass erase before unlock',
            default = None)

    parser.add_argument(
            '--bdaddr', '-b',
            help = 'BDADDR of nolleblinkmojt',
            default = None)

    parser.add_argument(
            '--nopass', '-n',
            help = 'Use 32*0xff as BSL password',
            action = 'store_true')

    subparsers = parser.add_subparsers(dest = 'action', help = 'Run mspbsl {command} -h for additional help')

    subparsers.add_parser('info', help = 'Show device id and BSL version')
    subparsers.add_parser('reset', help = 'Perform a hard reset')
    subparsers.add_parser('erase', help = 'Perform a mass erase')
    subparsers.add_parser('dumpcal', help = 'Dump DCO calibration data')
    
    parser_flash = subparsers.add_parser('flash', help = 'Write firmware to flash')
    parser_flash.add_argument('filename', help = 'Ihex file to write to flash')

    args = parser.parse_args()

    # Use the results from argparse to do the actual work...
    driver = None
    proto = None

    if args.driver == 'ft232':
        driver = FT232Driver()
    elif args.driver == 'nolle':
        if args.bdaddr:
            driver = NolleDriver(args.bdaddr)
        else:
            driver = NolleDriver()
    else:
        print 'Unknown driver %s!' % args.driver

    if args.protocol == '2':
        proto = BSL2xx(driver)
    elif args.protocol == '5':
        proto = BSL5xx(driver)
    else:
        print 'Unknown protocol %s!' % args.protocol

    # If we're just going to do a hard reset, then we won't have to enter the bootloader
    if args.action == 'reset':
        proto.reset()
        terminal(driver)
        sys.exit(0)

    proto.enter_bsl()

    if args.nopass:
        proto.unlock()
        if isinstance(driver, NolleDriver):
            print 'Checking calibration block'
            if driver.read_calibration() is None:
                print 'No calibration found.'
                cal = proto.read_calibration()
                print 'Read calibration from MSP'
                driver.write_calibration(cal)
                print 'Wrote calibration to IRMA'
            else:
                print 'Calibration found'
    elif args.password is not None:
        proto.unlock(unhexlify(args.password))
    else:
        proto.mass_erase()
        proto.unlock()

        if isinstance(driver, NolleDriver):
            print 'Checking calibration block'
            cal = driver.read_calibration()
            if cal is None:
                print 'Warning: No calibration found!'
            else:
                print 'Writing calibration to MSP'
                proto.write(0x10f6, cal)
                print 'Calibration written'

    if args.action == 'info':
        proto.check_version()

    elif args.action == 'erase':
        proto.mass_erase()

    elif args.action == 'flash':
        ihex = IHex()
        ihex.load(args.filename)
        size = ihex.condense()
        written = 0
        print 'Writing %s (%d bytes) to flash:' % (args.filename, size)
        for rec in ihex.data:
            if rec['type'] == 0:
                addr = rec['addr']
                data = rec['data']
                while len(data) > 0:
                    to_write = min(len(data), 250)
                    proto.write(addr, data[:to_write])
                    data = data[to_write:]
                    addr += to_write
                    written += to_write
                    write_progress(written, size)
        print
    elif args.action == 'dumpcal':
        print ' '.join(map(hex, map(ord, proto.read_calibration())))

    proto.reset()

    terminal(driver)
