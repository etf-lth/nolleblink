/*
 *  spi.h
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#ifndef SPI_H
#define SPI_H

void spiInit(void);
unsigned char spiTransfer(unsigned char);
unsigned char spiTransferFast(unsigned char);

#endif
