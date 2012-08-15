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
 * P2 6    Radio GDO0
 * P2 7    Bluetooth TX
 * P3 0    Radio /CS
 * P3 1..3 SPI
 * P4 3..4 LED Row
 * P4 5    Bluetooth RX
 * P4 6    LED /CS
 */

#define RADIO_CSn_PxOUT      P3OUT
#define RADIO_CSn_PxDIR      P3DIR
#define RADIO_CSn_PIN        BIT0

#define RADIO_GDO0_PxSEL     P2SEL
#define RADIO_GDO0_PxIN      P2IN
#define RADIO_GDO0_PxDIR     P2DIR
#define RADIO_GDO0_PxIES     P2IES
#define RADIO_GDO0_PxIFG     P2IFG
#define RADIO_GDO0_PxIE      P2IE
#define RADIO_GDO0_PIN       BIT6

#define BT_RX_PxSEL          P2SEL
#define BT_RX_PxDIR          P2DIR
#define BT_RX_PxIN           P2IN
#define BT_RX_PxIES          P2IES
#define BT_RX_PxIFG          P2IFG
#define BT_RX_PxIE           P2IE
#define BT_RX_PIN            BIT7
#define BT_TX_PxDIR          P4DIR
#define BT_TX_PxOUT          P4OUT
#define BT_TX_PIN            BIT5

#define LED_CSn_PxOUT        P4OUT
#define LED_CSn_PIN          BIT6

#define UART_PxDIR           P3DIR
#define UART_PxSEL           P3SEL
#define UART_TX_PIN          BIT4
#define UART_RX_PIN          BIT5

#endif
