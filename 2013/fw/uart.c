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


FIFO_EXTERN(bt_rx);
extern volatile int isr_rx, isr_break;

void uart_init(void)
{
    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCSSEL_2 | UCBRKIE;

    UCA0BR0 = _BR0_9600;
    UCA0BR1 = _BR1_9600;

    UCA0MCTL = UCBRS_2;

    UART_PxSEL |= UART_TX_PIN | UART_RX_PIN;
    UART_PxSEL2 |= UART_TX_PIN | UART_RX_PIN;
    UART_PxDIR |= UART_TX_PIN;
    UART_PxDIR &= ~UART_RX_PIN;

    UCA0CTL1 &= ~UCSWRST;

    IE2 |= UCA0RXIE;
}

#if 0
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
#endif

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

void uart_putbrk(void)
{
    UCA0CTL1 |= UCTXBRK;
    uart_putch(0);
}

#if 0
void uart_puthex(unsigned char c)
{
    const char hex[] = "0123456789abcdef";
    uart_putch(hex[c >> 4]);
    uart_putch(hex[c & 0xf]);
}
#endif

__attribute__((interrupt (USCIAB0RX_VECTOR)))
void uart_rxisr(void)
{
    if (IFG2 & UCA0RXIFG) {
        if (UCA0STAT & UCBRK) {
            char dummy = UCA0RXBUF;
            (void)dummy;
            isr_break = 1;
            __bic_SR_register_on_exit(LPM1_bits);
        } else {
            char ch = UCA0RXBUF;
            FIFO_PUT(bt_rx, ch);
            isr_rx = 1;
            __bic_SR_register_on_exit(LPM1_bits);
        }
    }
}

