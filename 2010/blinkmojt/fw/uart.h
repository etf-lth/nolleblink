/*
 *  uart.h
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#ifndef UART_H
#define UART_H

// 16MHz
#define _BR0_9600 0x82
#define _BR1_9600 0x06

void uartInit(void);
char uartGetChar(void);
void uartPutChar(char);
void uartPutString(const char *c);

extern void uartByteReceived(char);

#endif
