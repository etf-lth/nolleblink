/*
 *  uart.c
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#include <msp430.h>

#include "uart.h"
#include "board.h"

void uartInit(void)
{
    UART_PxSEL |= UART_TX_PIN | UART_RX_PIN;
    UART_PxDIR |= UART_TX_PIN;
    UART_PxDIR &= ~UART_RX_PIN;

    UCA0CTL1 = UCSSEL_2;

    UCA0BR0 = _BR0_9600;
    UCA0BR1 = _BR1_9600;

    UCA0MCTL = UCBRS_2;
    UCA0CTL1 &= ~UCSWRST;

    IE2 |= UCA0RXIE;
}

char uartGetChar(void)
{
    return UCA0RXBUF;
}

void uartPutChar(char c)
{
    UCA0TXBUF = c;
    while (!(IFG2 & UCA0TXIFG)) ;
}

void uartPutString(const char *c)
{
    while (*c) {
        uartPutChar(*c++);
    }
}

void uartPutByte(unsigned char c)
{
    const char hex[] = "0123456789abcdef";
    uartPutChar(hex[c >> 4]);
    uartPutChar(hex[c & 0xf]);
}

void __attribute__ ((weak)) uartByteReceived(char c)
{
}

__attribute__((interrupt (USCIAB0RX_VECTOR)))
void uartISR (void)
{
    char ch = UCA0RXBUF;
    uartByteReceived(ch);
}

