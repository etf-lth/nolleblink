#!/usr/bin/python
#
# (c) 2013, Fredrik Ahlberg <fredrik@z80.se>
#

import sys
import struct
from binascii import unhexlify
import subprocess

class IHex:

    def __init__(self):
        self.data = []

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

    def save(self, filename):
        f = file(filename, 'w')

        for l in self.data:
            checksum = (0x100 - (len(l['data']) + (l['addr'] >> 8) + (l['addr'] & 0xff) +
                + l['type'] + sum(map(ord, l['data']))) & 0xff) & 0xff
            f.write(":%02x%04x%02x%s%02x\n" % (len(l['data']), l['addr'], l['type'],
                ''.join(map(lambda x : '%02x' % x, map(ord, l['data']))),
                checksum))

        f.close()

    def append(self, type, addr, data):
        self.data.append({'type': type, 'addr': addr, 'data': data})

class SbwConfig:

    def gen_conf(self, text):
        text = text.decode('utf8').encode('latin_1')
        text += '\0'*(126-len(text))
        checksum = sum(map(ord, text))
        text += chr(checksum & 0xff) + chr(checksum >> 8)
        return text

    def gen_hex(self, text):
        conf = self.gen_conf(text)

        ihx = IHex()
        ihx.load('blink13.hex')
        addr = 0x1000
        while len(conf) > 0:
            t = conf[:16]
            conf = conf[16:]
            ihx.append(0, addr, t)
            addr += 16

        ihx.save('temp.hex')

    def run(self, text):
        self.gen_hex(text)
        subprocess.call(["mspdebug","rf2500","erase segment 0x1000","erase segment 0x1040","prog temp.hex","exit"])

if __name__ == '__main__':
    sbw = SbwConfig()
    if len(sys.argv) < 2:
        print 'sbwconf: expected name!'
    else:
        sbw.run(sys.argv[1]+'   ')
