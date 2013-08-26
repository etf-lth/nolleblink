/*
 *  board.h
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#ifndef BOARD_H
#define BOARD_H

#define LED_CSn_PxOUT   P1OUT
#define LED_CSn_PxDIR   P1DIR
#define LED_CSn_PIN     BIT0

#define LED_ROW_PxDIR   P2DIR
#define LED_ROW_PxOUT   P2OUT
#define LED_ROW_PxSEL   P2SEL
#define LED_ROW_PxSEL2  P2SEL2
#define LED_ROW_PINS    BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6

#define SPI_PxDIR       P1DIR
#define SPI_PxOUT       P1OUT
#define SPI_PxSEL       P1SEL
#define SPI_PxSEL2      P1SEL2
#define SPI_SCK_PIN     BIT5
#define SPI_MOSI_PIN    BIT7
#define SPI_MISO_PIN    BIT6

#define UART_PxSEL      P1SEL
#define UART_PxSEL2     P1SEL2
#define UART_PxDIR      P1DIR
#define UART_PxOUT      P1OUT
#define UART_TX_PIN     BIT2
#define UART_RX_PIN     BIT1

#endif
