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

void uart_init(void);
void uart_disable(void);
char uart_getch(void);
void uart_putch(char);
void uart_puthex(unsigned char);
void uart_puts(const char *c);
void uart_putbrk(void);

extern void uart_received(char);
extern void uart_break(char);

#endif
