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

/* Pin usage 
 *
 * P1 0..2 On-board LED + button
 * P2 0..4 LED Row
 * P2 6..7 Radio GDOx
 * P3 0    Radio /CS
 * P3 1..3 SPI
 * P4 3..4 LED Row
 * P4 6    LED /CS
 */

#define RADIO_CSn_PxOUT      P3OUT
#define RADIO_CSn_PxDIR      P3DIR
#define RADIO_CSn_PIN        BIT0

#define RADIO_GDOx_PxSEL     P2SEL
#define RADIO_GDOx_PxIN      P2IN
#define RADIO_GDOx_PxDIR     P2DIR
#define RADIO_GDOx_PxIES     P2IES
#define RADIO_GDOx_PxIFG     P2IFG
#define RADIO_GDOx_PxIE      P2IE
#define RADIO_GDO0_PIN       BIT6
#define RADIO_GDO2_PIN       BIT7

#define LED_CSn_PxOUT        P4OUT
#define LED_CSn_PIN          BIT6

#define UART_PxDIR           P3DIR
#define UART_PxSEL           P3SEL
#define UART_TX_PIN          BIT4
#define UART_RX_PIN          BIT5

#endif
