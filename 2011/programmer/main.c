/*
 *  main.c
 *  Nolleblink 2011 | Programmerare
 *
 *  Created by Fredrik Ahlberg on 2011-08-23.
 *  Copyright (c) 2011 ETF. All rights reserved.
 *
 */

#include <io.h>
#include <signal.h>
#include <string.h>

#include "spi.h"
#include "radio.h"
#include "uart.h"
#include "protocol.h"

void calcChecksum(char *buf)
{
    packet_t *pkt = (packet_t *)buf;

    unsigned short idx, checksum = 0x1337;
    for (idx=0; idx<(sizeof(packet_t)-sizeof(unsigned short)); idx++)
        checksum += buf[idx];

    pkt->checksum = checksum;
}

void uartByteReceived(char c)
{
    static unsigned char idx, n, buf[60], tmp;

    if (c == '*') {
        n = idx = 0;
        return;
    }

    if (c == '\r') {
        calcChecksum(buf);
        radioTransmitPacket(buf);
        idx = n = 0;
        return;   
    }

    if (c >= 'a' && c <= 'f')
        tmp = c - 'a' + 10;
    else if (c >= '0' && c <= '9')
        tmp = c - '0';
    else
        return;

    if (!n) {
        buf[idx] = tmp << 4;
        n++;
    } else {
        buf[idx] |= tmp;
        n = 0;
        idx++;
    }
    
  /*packet_t pkt = {
    .to = {0, 0},
    .from = {0, 0},
    .type = CMD_WRITE_ID,
    .size = 2,
    .payload = {0x27, 0x0f} //2, 'l', 'u', 'l', 'z', 0, 0xff, 0xaa, 0x55}
  };

  radioTransmitPacket((char *)&pkt);*/

  //uartPutString("Nolleblink 2010 ");
}

void radioPacketReceived(char *buf, char len)
{
    unsigned short idx;
	for (idx=0; idx<len; idx++) {
	    char tmp = buf[idx] >> 4;
	    if(tmp > 9)
	      tmp = 'a' + tmp - 10;
	    else
	      tmp = '9' + tmp - 9;

	    tmp = buf[idx] & 0x0f;
	    if(tmp > 9)
	      tmp = 'a' + tmp - 10;
	    else
	      tmp = '9' + tmp - 9;
        
        uartPutChar(tmp);
	}

    uartPutChar('\n');
    uartPutChar('\r');

	return;
}

int main(void)
{
  // Disable watchdog
  WDTCTL = WDTPW | WDTHOLD;

  // Set DCO frequency
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;
  BCSCTL2 &= ~(BIT5|BIT4);  // Set MCLK prescaler to 1

  // Initialize SPI controller
  spiInit();

  // Init radio
  radioInit();

  // Init debug UART
  uartInit();

  // Enable interrupts and go to sleep
  __bis_SR_register((LPM1_bits)|(GIE));

  // we won't get here...
  return 0;
}
