#!/usr/bin/python
# coding=utf-8
import sys
import serial
from PyQt4 import QtGui,QtCore
import mainui

class NolleConfig(QtGui.QMainWindow):
    
    CMD_RESERVED_1 = 0
    CMD_PING = 1
    CMD_TEMP_MESSAGE = 2
    CMD_SHOW_ID = 3
    CMD_SYNC = 4
    CMD_PONG = 5
    CMD_RESERVED_2 = 6
    CMD_WRITE_TEXT = 7
    CMD_WRITE_TEXT_ID_OLD = 8
    CMD_VIRUS = 9
    CMD_WRITE_ID = 10
    CMD_FRAMEBUFFER = 11
    CMD_HEARTBEAT = 12
    CMD_READ_TEXT = 13

    def __init__(self):
        QtGui.QMainWindow.__init__(self)

        self.ui = mainui.Ui_MainWindow()
        self.ui.setupUi(self)
        self.setWindowTitle('ETF Nolleblink 2011')

        self.serial = serial.Serial('/dev/ttyUSB0', timeout=0.1)
        self.ui.showButton.clicked.connect(self.showId)
        self.ui.initButton.clicked.connect(self.initBlinkmojt)
        self.ui.writeButton.clicked.connect(self.writeText)
        self.ui.scrollButton.clicked.connect(self.scrollText)

        self.db = open("nollor.txt","a")

    def showId(self):
        print "show id"
        self.sendPacket(0, self.CMD_SHOW_ID, [])

    def initBlinkmojt(self):
        print "init"
        uid = int(self.ui.idEdit.text())
        text = str(self.ui.textEdit.text())
        #text = string.replace(text, "å", "\xe5")
        stil = str(self.ui.stilEdit.text())
        #dst = int(self.ui.dstEdit.text())
        self.db.write("%d\t%s\t%s\n" % (uid, text, stil))
        self.db.flush()
        text = text + "   "
        self.sendPacket(9999, self.CMD_WRITE_ID, [(uid >> 8), uid & 0xff])
        self.sendPacket(9999, self.CMD_WRITE_ID, [(uid >> 8), uid & 0xff])
        self.sendPacket(9999, self.CMD_WRITE_ID, [(uid >> 8), uid & 0xff])
        self.sendPacket(9999, self.CMD_WRITE_ID, [(uid >> 8), uid & 0xff])
        #text = text.encode(latin_1)
        self.sendPacket(uid, self.CMD_WRITE_TEXT, map(ord, text))
        self.sendPacket(uid, self.CMD_WRITE_TEXT, map(ord, text))
        self.sendPacket(uid, self.CMD_WRITE_TEXT, map(ord, text))
        self.sendPacket(uid, self.CMD_WRITE_TEXT, map(ord, text))
        self.ui.idEdit.setText(str(uid + 1))

    def writeText(self):
        print "write"
        text = str(self.ui.textEdit.text())
        dst = int(self.ui.dstEdit.text())
        text = text + "   "
        self.sendPacket(dst, self.CMD_WRITE_TEXT, map(ord, text))

    def scrollText(self):
        print "scroll"
        text = str(self.ui.textEdit.text())
        dst = int(self.ui.dstEdit.text())
        text = "\x02" + text + "   "
        self.sendPacket(dst, self.CMD_TEMP_MESSAGE, map(ord, text))

    def sendPacket(self, to, cmd, data):
        pkt = [0]
        pkt.append((to >> 8) & 0xff)
        pkt.append(to & 0xff)
        pkt.append(0)
        pkt.append(0)
        pkt.append(0)
        pkt.append(0)
        pkt.append(cmd)
        pkt.append(len(data))
        pkt.extend(data)

        s = '*'
        for c in pkt:
            s = s + "%02x" % c
        s = s + '\r'
        print pkt
        #print ('*' + ''.join(map(hex, pkt)) + "\r")
        #print s
        self.serial.write(s)
        
        

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    nolla = NolleConfig()
    nolla.show()
    sys.exit(app.exec_())
