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
#include "board.h"

void spi_init(void)
{
    UCB0CTL1 |= UCSWRST;                    // Disable USCI state machine
    UCB0CTL0 |= UCMST|UCCKPH|UCMSB|UCSYNC;  // 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                   // SMCLK
    UCB0BR0 = 1;                            // UCLK/1
    UCB0BR1 = 0;

    SPI_PxDIR |= SPI_SCK_PIN|SPI_MOSI_PIN;  // SPI SCK and MOSI as outputs
    SPI_PxSEL |= SPI_SCK_PIN|SPI_MOSI_PIN;
    SPI_PxSEL2 |= SPI_SCK_PIN|SPI_MOSI_PIN;

    UCB0CTL1 &= ~UCSWRST;                   // Enable USCI state machine
}


unsigned char spi_xfer(unsigned char data)
{
    UCB0TXBUF = data;
    while (!(IFG2 & UCB0RXIFG));
    return UCB0RXBUF;
}
