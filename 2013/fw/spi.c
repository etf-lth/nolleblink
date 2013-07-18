/*
 *  spi.c
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#include <msp430.h>

#include "spi.h"

void spiInit(void)
{
    UCB0CTL1 |= UCSWRST;                      // Disable USCI state machine
    UCB0CTL0 |= UCMST|UCCKPH|UCMSB|UCSYNC;    // 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                     // SMCLK
    UCB0BR0 = 1;                              // UCLK/1 (über!)
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Enable USCI state machine

    P1OUT |= BIT0;
    P1DIR |= BIT0|BIT5|BIT7;                     // SPI TXD out direction
    P1SEL |= BIT5|BIT7;
    P1SEL2 |= BIT5|BIT7;
}

unsigned char spiTransfer(unsigned char data)
{
    UCB0BR0 = 6;                             // UCLK/16 (not so über.)
    UCB0TXBUF = data;
    while (!(IFG2 & UCB0RXIFG));
    return UCB0RXBUF;
}

unsigned char spiTransferFast(unsigned char data)
{
    UCB0BR0 = 1;                              // UCLK/1 (über!)
    UCB0TXBUF = data;
    while (!(IFG2 & UCB0RXIFG));
    return UCB0RXBUF;
}
