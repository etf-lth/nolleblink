#!/usr/bin/env python

from bluetooth.bluez import DeviceDiscoverer
from threading import Thread
from mspbsl import NolleDriver, BSL2xx, IHex, write_progress
import time

class BlinkMojt(Thread):

    def __init__(self, address):
        Thread.__init__(self)
        self.addr = address
        self.daemon = True
        self.visible = True
        self.flashed = False
        self.failed = False
        self.last_seen = 0
        self.start()

    def run(self):
        print 'BlinkMojt(%s) thread starting' % self.addr
        driver = NolleDriver(self.addr)
        print 'Got driver',driver
        proto = BSL2xx(driver)
        print 'Got proto',proto
        proto.enter_bsl()
        proto.unlock()
        print 'Checking calibration'
        if driver.read_calibration() is None:
            print 'No calibration found, reading from MSP'
            cal = proto.read_calibration()
            print 'Writing calibration to IRMA'
            driver.write_calibration(cal)
        else:
            print 'Calibration found'
            cal = driver.read_calibration()

        ihex = IHex()
        ihex.load('blink13.hex')
        size = ihex.condense()
        written = 0

        print 'Writing firmware (%d bytes) to flash:' % size
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
        
        proto.reset()

        print 'Done!'


class MojtDiscoverer(DeviceDiscoverer, Thread):

    def __init__(self):
        DeviceDiscoverer.__init__(self)
        Thread.__init__(self)
        self.daemon = True

        self.devices = {}

    def start_inquiry(self):
        self.find_devices(duration=4, lookup_names=False)

    def device_discovered(self, address, device_class, name):
        #print address
        if address in self.devices:
            print 'Still seeing', address
        else:
            print 'New device', address
            self.devices[address] = BlinkMojt(address)

    def inquiry_complete(self):
        self.start_inquiry()

    def run(self):
        self.start_inquiry()
        while True:
            md.process_event()

md = MojtDiscoverer()
md.start()
while True:
    time.sleep(.1)
