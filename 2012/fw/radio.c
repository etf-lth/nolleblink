/*
 *  radio.c
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-25.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#include <msp430.h>

#include "radio.h"
#include "spi.h"
#include "board.h"

void radioDelay(unsigned int);
void radioReset(void);
void radioWriteSettings(void);
char radioCommand(char);
void radioWriteRegister(char, char);
void radioWriteRegisters(char, const char *, char);
char radioReadRegister(char);
void radioReadRegisters(char, char *, char);
char radioReadStatus(char);
void radioTransmitPacket(char *);
char radioReceivePacket(char *);

const char radioConfig[][2] = {
    {CC2500_IOCFG2,     0x2E},
    {CC2500_IOCFG0,     0x06},
    {CC2500_PKTLEN,     PKTLEN},
    {CC2500_PKTCTRL1,   0x44},
    {CC2500_PKTCTRL0,   0x44},
    {CC2500_ADDR,       0x00},
    {CC2500_CHANNR,     0x30},
    {CC2500_MDMCFG2,    0x73},
    {CC2500_MDMCFG1,    0xC2},
    {CC2500_MCSM1,      0x0F},
    {CC2500_MCSM0,      0x18}
};

// PATABLE (+1 dBm output power)
const char radioPATable[] = {0xFF};
const char radioPATableLen = 1;

char radioBuffer[PKTLEN];

void radioInit(void)
{
    // Radio Chip Select pin as output
    RADIO_CSn_PxDIR |= RADIO_CSn_PIN;
    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;

    // Sikrit Blinkenlights...
    P1OUT &= 0xfc;
    P1DIR |= 0x03;

    // Initialize CC2500
    radioReset();
    radioWriteSettings();

    // Radio GDO0 pin as input with interrupt on falling edge
    RADIO_GDO0_PxSEL &= ~RADIO_GDO0_PIN;
    RADIO_GDO0_PxDIR &= ~RADIO_GDO0_PIN;
    RADIO_GDO0_PxIES |= RADIO_GDO0_PIN;
    RADIO_GDO0_PxIFG &= ~RADIO_GDO0_PIN;
    RADIO_GDO0_PxIE |= RADIO_GDO0_PIN;

    // Switch to RX mode
    radioCommand(CC2500_SRX);
}

void radioDelay(unsigned int cycles)
{
    while (cycles > 15) {
        cycles -= 6;
    }
}

void radioReset(void)
{
    // Pulse /CS
    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;
    radioDelay(30);
    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;
    radioDelay(30);
    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;
    radioDelay(45);

    // Issue a soft reset
    radioCommand(CC2500_SRES);

    radioDelay(2000);
}

void radioWriteSettings(void)
{
    unsigned int idx;

    for (idx=0; idx<sizeof(radioConfig)/sizeof(char[2]); idx++) {
        radioWriteRegister(radioConfig[idx][0], radioConfig[idx][1]);
    }

    radioWriteRegisters(CC2500_PATABLE, radioPATable, radioPATableLen);
}

void __attribute__ ((weak)) radioPacketReceived(char *buf, char len)
{
}

void radioISR(void)
{
    if (radioReceivePacket(radioBuffer/*, &len*/)) {
        //P1OUT ^= 1;
	    radioPacketReceived(radioBuffer, PKTLEN);
    }
}

char radioCommand(char cmd)
{
    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;
    char tmp = spiTransfer(cmd);
    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;

    return tmp;
}

void radioWriteRegister(char addr, char val)
{
    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;
    spiTransfer(addr);
    spiTransfer(val);
    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;
}

void radioWriteRegisters(char addr, const char *vals, char count)
{
    unsigned int idx;

    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;
    spiTransfer(addr | CC2500_WRITE_BURST);

    for (idx=0; idx<count; idx++) {
        spiTransfer(vals[idx]);
    }

    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;
}

char radioReadRegister(char addr)
{
    char val;

    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;

    spiTransfer(addr | CC2500_READ_SINGLE);
    val = spiTransfer(0);

    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;

    return val;
}

void radioReadRegisters(char addr, char *buf, char count)
{
    unsigned int idx;

    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;

    spiTransfer(addr | CC2500_READ_BURST);

    for (idx=0; idx<count; idx++) {
        buf[idx] = spiTransfer(0);
    }

    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;
}

char radioReadStatus(char addr)
{
    char status;

    RADIO_CSn_PxOUT &= ~RADIO_CSn_PIN;

    spiTransfer(addr | CC2500_READ_BURST);
    status = spiTransfer(0);

    RADIO_CSn_PxOUT |= RADIO_CSn_PIN;

    return status;
}

void radioTransmitPacket(char *buf)
{
    //P1OUT ^= 2;

    // Fill TX Buffer
    radioWriteRegisters(CC2500_TXFIFO, buf, PKTLEN);

    // Start transmission
    radioCommand(CC2500_STX);

    // Wait for the transmission to complete
    while (!(RADIO_GDO0_PxIN & RADIO_GDO0_PIN)) ;
    while (RADIO_GDO0_PxIN & RADIO_GDO0_PIN) ;
    RADIO_GDO0_PxIFG &= ~RADIO_GDO0_PIN;

    //radioCommand(CC2500_SRX);
}

char radioReceivePacket(char *buf/*, char *len*/)
{
    char status[2]; //, pktLen;

    // Packet in FIFO?
    unsigned char len = radioReadStatus(CC2500_RXBYTES) & CC2500_NUM_RXBYTES;
    if (len >= PKTLEN) {
        // Check the packet length
        //pktLen = radioReadRegister(CC2500_RXFIFO);

        //if(pktLen <= 64 && pktLen <= *len)
        //{
        // Read the packet and the appended status.
        radioReadRegisters(CC2500_RXFIFO, buf, PKTLEN /*pktlen*/);
        //*len = 32; //pktLen;
        radioReadRegisters(CC2500_RXFIFO, status, 2);
        radioCommand(CC2500_SIDLE);
        radioCommand(CC2500_SFRX);
        radioCommand(CC2500_SRX);
        return status[CC2500_LQI_RX] & CC2500_CRC_OK;
        //return 1;
        //}
        //else
        //{
        // Packet didn't fit in buffer, flush the FIFO.
        //*len = pktLen;
        //radioCommand(CC2500_SFRX);
        //return 0;
        //}
    } else {
        //radioCommand(CC2500_SFRX);
        return 0;
    }
}

