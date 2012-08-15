/*
 *  main.c
 *  Nolleblink 2012 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010-2012 ETF. All rights reserved.
 *
 */

#include <msp430.h>
#include <string.h>

#include "spi.h"
#include "led.h"
#include "radio.h"
#include "uart.h"
#include "flash.h"
#include "protocol.h"
#include "board.h"
#include "fifo.h"

typedef struct {
    unsigned char id[2];
    unsigned char text[40];
} config_t;

config_t config;
unsigned char infection_rate, cure_rate;
volatile unsigned char isr_radio, isr_timer, isr_uart;

FIFO_EXTERN(rx_fifo);

//void uartByteReceived(char c)
//{
  /*packet_t pkt = {
    .to = {0, 0},
    .from = {0, 0},
    .type = CMD_WRITE_ID,
    .size = 2,
    .payload = {0x27, 0x0f} //2, 'l', 'u', 'l', 'z', 0, 0xff, 0xaa, 0x55}
  };

  radioTransmitPacket((char *)&pkt);*/

  //uartPutString("Nolleblink 2010 ");
  //}

void calcChecksum(char *buf)
{
    packet_t *pkt = (packet_t *)buf;

    unsigned short idx, checksum = 0x1337;
    for (idx=0; idx<(sizeof(packet_t)-sizeof(unsigned short)); idx++) {
        checksum += buf[idx];
    }

    pkt->checksum = checksum;
}

void radioPacketReceived(char *buf, char len)
{
    packet_t *pkt = (packet_t *)buf;

    if ((pkt->to[0] || pkt->to[1])
        &&(pkt->to[0] != config.id[0] || pkt->to[1] != config.id[1])) {
        return;
    }

    unsigned short idx, checksum = 0x1337;
    for (idx=0; idx<(sizeof(packet_t)-sizeof(unsigned short)); idx++) {
        checksum += buf[idx];
    }
    
    if (pkt->checksum != checksum) {
        return;
    }

    switch (pkt->type) {
    case CMD_PING:
        pkt->to[0] = pkt->from[0];
        pkt->to[1] = pkt->from[1];
        pkt->from[0] = config.id[0];
        pkt->from[1] = config.id[1];
        pkt->type = CMD_PONG;
        calcChecksum(buf);
        radioTransmitPacket(buf);
        break;

    case CMD_TEMP_MESSAGE:
        if (pkt->size < sizeof(pkt->payload)) {
	        pkt->payload[pkt->size] = 0;
        } else {
	        pkt->payload[sizeof(pkt->payload)-1] = 0;
        }
        ledSetTempText(pkt->payload[0], &pkt->payload[1]);
        break;

    case CMD_SHOW_ID:
        {
            if (pkt->size) {
	            return;
            }

            unsigned char buf[8], idx;
            unsigned short id = config.id[0] << 8 | config.id[1];

            for (idx=0; idx<8; idx++) {
	            char tmp = id >> 12;
	            if (tmp > 9) {
	                buf[idx] = 'a' + tmp - 10;
                } else {
	                buf[idx] = '9' + tmp - 9;
                }

                id <<= 4;
            }

            buf[4] = buf[5] = buf[6] = ' ';
            buf[7] = 0;
            ledSetTempText(3, buf);
        }
        break;

    case CMD_WRITE_TEXT:
        if (pkt->size < sizeof(pkt->payload)) {
	        pkt->payload[pkt->size] = 0;
        } else {
	        pkt->payload[sizeof(pkt->payload)-1] = 0;
        }
        strncpy(config.text, pkt->payload, sizeof(config.text));
        flashWrite((const char *)&config, sizeof(config));
        ledSetState(LED_STATE_SCROLL_TEXT);
        ledSetText(config.text);
        break;

    case CMD_WRITE_ID:
        //ledSetTempText(1, "raj raj...   ");
        if (pkt->size != 2) {
	        return;
        }
        config.id[0] = pkt->payload[0];
        config.id[1] = pkt->payload[1];
        flashWrite((const char *)&config, sizeof(config));
        ledSetTempText(1, "Nytt ID.   ");
        break;

    case CMD_FRAMEBUFFER:
        if (pkt->size != 15) {
	        return;
        }
        ledSetState(LED_STATE_FRAMEBUFFER);
        ledSetBuffer(pkt->payload);
        break;

    case CMD_READ_TEXT:
        if (pkt->size) {
	        return;
        }

        pkt->to[0] = pkt->from[0];
        pkt->to[1] = pkt->from[1];
        pkt->from[0] = config.id[0];
        pkt->from[1] = config.id[1];
        pkt->type = CMD_READ_TEXT;

        pkt->size = strlen(config.text);
        strncpy(pkt->payload, config.text, sizeof(pkt->payload));

        if (pkt->size) {
            calcChecksum(buf);
	        radioTransmitPacket(buf);
        }
        break;

    case CMD_HEARTBEAT:
        if (pkt->payload[0]) {
	        if (pkt->payload[0] > (TAR % 100)) {
	            infection_rate = pkt->payload[0];
	            cure_rate = pkt->payload[1];

	            ledSetState(LED_STATE_VIRUS);
            }
        }
        break;

    case CMD_VIRUS:
        infection_rate = pkt->payload[0];
        cure_rate = pkt->payload[1];
        ledSetState(LED_STATE_VIRUS);
        break;
    }
}

void heartTimerISR(void)
{
    TBR = 0;

    static short div = 0;
    if (div++ < 100) {
        return;
    }
    div = 0;

    // 1 Hz
    unsigned short time = flashGetTime();

    packet_t pkt = {
        .type = CMD_HEARTBEAT,
        .from = {config.id[0], config.id[1]},
        .payload = {infection_rate, cure_rate, (time >> 8) & 0xff, time & 0xff}
    };

    calcChecksum((char *)&pkt);
    radioTransmitPacket((char *)&pkt);

    if (cure_rate > (TAR % 100)) {
        infection_rate = 0;
        ledSetState(LED_STATE_SCROLL_TEXT);
    }

    static short rtdiv = 0;
    if (rtdiv++ < 60) {
        return;
    }
    rtdiv = 0;

    flashUpdateTime();
}

__attribute__((interrupt (PORT2_VECTOR)))
void PORT2_ISR(void)
{
    softu_interrupt();
}

__attribute__((interrupt (TIMERA0_VECTOR)))
void TIMERA_ISR(void)
{
    isr_timer = 1;
    __bic_SR_register_on_exit(LPM1_bits);
}

int main(void)
{
    // Disable watchdog
    WDTCTL = WDTPW | WDTHOLD;

    // Set DCO frequency
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
    BCSCTL2 &= ~(BIT5|BIT4);  // Set MCLK prescaler to 1

    // Initialize SPI controller
    spiInit();

    // Init LED display
    //ledInit();

    infection_rate = cure_rate = 0;

#if 0
    // Read configuration
    if (flashRead((char *)&config, sizeof(config))) {
        if (!config.text[0]) {
	        ledSetState(LED_STATE_SCROLL_BARS);
        } else {
	        ledSetText(config.text);
	        ledSetState(LED_STATE_SCROLL_TEXT);
        }
    } else {
        config.id[0] = 0x27;
        config.id[1] = 0x0f;
        ledSetState(LED_STATE_SCROLL_BARS);
    }
#endif

    // Init radio
    radioInit();

    // Init debug UART
    uartInit();
    uartPutString("Nolleblink 2012\n\r");
    uartPutString("(c) ElektroTekniska Föreningen, 2009-2012\n\r");

    // System tick at 400 Hz
    TACCTL0 = CCIE;
    TACCR0 = 5000;
    TACTL = TASSEL_2|MC_1|ID_3;

    // Initialize Soft UART
    softu_init();
    softu_transmit('@');

    int div = 0;

    while (1) {
        // Enable interrupts and go to sleep
        __bis_SR_register((LPM1_bits)|(GIE));

        // Something waked us up, handle it
        if (isr_timer) {
            if (++div == 400) {
                P1OUT ^= 1;
                div = 0;
            }
            isr_timer = 0;
        }

        if (isr_uart) {
            isr_uart = 0;

            while (!FIFO_EMPTY(rx_fifo)) {
                uartPutChar(FIFO_GET(rx_fifo));
            }

            P1OUT ^= 2;
        }
    }

    // we won't get here...
    return 0;
}

