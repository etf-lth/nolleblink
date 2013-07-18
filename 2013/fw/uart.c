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
#include "fifo.h"

//FIFO_DEF(bt_rx);

//extern volatile char isr_uart;

#define UART_PxSEL  P1SEL
#define UART_PxDIR  P1DIR
#define UART_PxOUT  P1OUT
#define UART_TX_PIN BIT2
#define UART_RX_PIN BIT1

void uart_init(void)
{
    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0 = _BR0_9600;
    UCA0BR1 = _BR1_9600;

    UCA0MCTL = UCBRS_2;

    UART_PxSEL |= UART_TX_PIN | UART_RX_PIN;
    UART_PxDIR |= UART_TX_PIN;
    UART_PxDIR &= ~UART_RX_PIN;

    UCA0CTL1 &= ~UCSWRST;

    IE2 |= UCA0RXIE;
}

void uart_disable(void)
{
    UART_PxSEL &= ~(UART_TX_PIN | UART_RX_PIN);
    UART_PxDIR &= ~(UART_TX_PIN | UART_RX_PIN);
    UART_PxOUT &= ~(UART_TX_PIN | UART_RX_PIN);
}

char uart_getch(void)
{
    return UCA0RXBUF;
}

void uart_putch(char c)
{
    UCA0TXBUF = c;
    while (!(IFG2 & UCA0TXIFG)) ;
}

void uart_puts(const char *c)
{
    while (*c) {
        uart_putch(*c++);
    }
}

void uart_puthex(unsigned char c)
{
    const char hex[] = "0123456789abcdef";
    uart_putch(hex[c >> 4]);
    uart_putch(hex[c & 0xf]);
}

void __attribute__ ((weak)) uart_received(char c)
{
}

__attribute__((interrupt (USCIAB0RX_VECTOR)))
void uart_rxisr(void)
{
    if (IFG2 & UCA0RXIFG) {
        char ch = UCA0RXBUF;
        uart_received(ch);
        /*FIFO_PUT(bt_rx, ch);
        isr_uart = 1;*/
    }
}

