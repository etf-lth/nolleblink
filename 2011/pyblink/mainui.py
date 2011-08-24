# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'main.ui'
#
# Created: Tue Aug 23 13:06:52 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(372, 238)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(MainWindow.sizePolicy().hasHeightForWidth())
        MainWindow.setSizePolicy(sizePolicy)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.textEdit = QtGui.QLineEdit(self.centralwidget)
        self.textEdit.setGeometry(QtCore.QRect(30, 0, 341, 24))
        self.textEdit.setObjectName(_fromUtf8("textEdit"))
        self.label = QtGui.QLabel(self.centralwidget)
        self.label.setGeometry(QtCore.QRect(0, 1, 31, 20))
        self.label.setObjectName(_fromUtf8("label"))
        self.stilEdit = QtGui.QLineEdit(self.centralwidget)
        self.stilEdit.setGeometry(QtCore.QRect(30, 29, 71, 24))
        self.stilEdit.setObjectName(_fromUtf8("stilEdit"))
        self.label_2 = QtGui.QLabel(self.centralwidget)
        self.label_2.setGeometry(QtCore.QRect(0, 30, 31, 20))
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.label_3 = QtGui.QLabel(self.centralwidget)
        self.label_3.setGeometry(QtCore.QRect(110, 31, 21, 20))
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.idEdit = QtGui.QLineEdit(self.centralwidget)
        self.idEdit.setGeometry(QtCore.QRect(130, 30, 41, 24))
        self.idEdit.setObjectName(_fromUtf8("idEdit"))
        self.initButton = QtGui.QPushButton(self.centralwidget)
        self.initButton.setGeometry(QtCore.QRect(0, 60, 51, 27))
        self.initButton.setObjectName(_fromUtf8("initButton"))
        self.writeButton = QtGui.QPushButton(self.centralwidget)
        self.writeButton.setGeometry(QtCore.QRect(60, 60, 61, 27))
        self.writeButton.setObjectName(_fromUtf8("writeButton"))
        self.scrollButton = QtGui.QPushButton(self.centralwidget)
        self.scrollButton.setGeometry(QtCore.QRect(130, 60, 51, 27))
        self.scrollButton.setObjectName(_fromUtf8("scrollButton"))
        self.showButton = QtGui.QPushButton(self.centralwidget)
        self.showButton.setGeometry(QtCore.QRect(190, 60, 61, 27))
        self.showButton.setObjectName(_fromUtf8("showButton"))
        self.virusButton = QtGui.QPushButton(self.centralwidget)
        self.virusButton.setGeometry(QtCore.QRect(260, 60, 51, 27))
        self.virusButton.setObjectName(_fromUtf8("virusButton"))
        self.consoleEdit = QtGui.QTextEdit(self.centralwidget)
        self.consoleEdit.setGeometry(QtCore.QRect(0, 90, 371, 101))
        self.consoleEdit.setObjectName(_fromUtf8("consoleEdit"))
        self.dstEdit = QtGui.QLineEdit(self.centralwidget)
        self.dstEdit.setGeometry(QtCore.QRect(210, 30, 41, 24))
        self.dstEdit.setObjectName(_fromUtf8("dstEdit"))
        self.label_4 = QtGui.QLabel(self.centralwidget)
        self.label_4.setGeometry(QtCore.QRect(180, 30, 31, 20))
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.label_5 = QtGui.QLabel(self.centralwidget)
        self.label_5.setGeometry(QtCore.QRect(260, 30, 21, 20))
        self.label_5.setObjectName(_fromUtf8("label_5"))
        self.infectEdit = QtGui.QLineEdit(self.centralwidget)
        self.infectEdit.setGeometry(QtCore.QRect(280, 30, 31, 24))
        self.infectEdit.setObjectName(_fromUtf8("infectEdit"))
        self.label_6 = QtGui.QLabel(self.centralwidget)
        self.label_6.setGeometry(QtCore.QRect(320, 30, 21, 20))
        self.label_6.setObjectName(_fromUtf8("label_6"))
        self.cureEdit = QtGui.QLineEdit(self.centralwidget)
        self.cureEdit.setGeometry(QtCore.QRect(340, 30, 31, 24))
        self.cureEdit.setObjectName(_fromUtf8("cureEdit"))
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 372, 22))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setSizeGripEnabled(False)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QtGui.QApplication.translate("MainWindow", "MainWindow", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("MainWindow", "Text:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("MainWindow", "StiL:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("MainWindow", "ID:", None, QtGui.QApplication.UnicodeUTF8))
        self.idEdit.setText(QtGui.QApplication.translate("MainWindow", "230", None, QtGui.QApplication.UnicodeUTF8))
        self.initButton.setText(QtGui.QApplication.translate("MainWindow", "Init", None, QtGui.QApplication.UnicodeUTF8))
        self.writeButton.setText(QtGui.QApplication.translate("MainWindow", "Write Text", None, QtGui.QApplication.UnicodeUTF8))
        self.scrollButton.setText(QtGui.QApplication.translate("MainWindow", "Scroll", None, QtGui.QApplication.UnicodeUTF8))
        self.showButton.setText(QtGui.QApplication.translate("MainWindow", "Show ID", None, QtGui.QApplication.UnicodeUTF8))
        self.virusButton.setText(QtGui.QApplication.translate("MainWindow", "E-Virus", None, QtGui.QApplication.UnicodeUTF8))
        self.dstEdit.setText(QtGui.QApplication.translate("MainWindow", "0", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("MainWindow", "Dst:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("MainWindow", "IR:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_6.setText(QtGui.QApplication.translate("MainWindow", "CR:", None, QtGui.QApplication.UnicodeUTF8))

