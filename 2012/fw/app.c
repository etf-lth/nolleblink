/*
 *  app.c
 *  Nolleblink 2012 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2012-08-20.
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
    union {
        unsigned char id[2];
        unsigned short id_s;
    };
    unsigned char text[40];
    unsigned char friendly_name[40];
} config_t;

config_t config;
unsigned char infection_rate, cure_rate;

void calcChecksum(char *buf)
{
    packet_t *pkt = (packet_t *)buf;

    unsigned short idx, checksum = 0x1337;
    for (idx=0; idx<(sizeof(packet_t)-sizeof(unsigned short)); idx++) {
        checksum += buf[idx];
    }

    pkt->checksum = checksum;
}

void appSetText(char *str)
{
    strncpy(config.text, str, sizeof(config.text));
    flashWrite((const char *)&config, sizeof(config));
    ledSetState(LED_STATE_SCROLL_TEXT);
    ledSetText(config.text);
}

void appSetFriendlyName(char *str)
{
    strncpy(config.friendly_name, str, sizeof(config.friendly_name));
    flashWrite((const char *)&config, sizeof(config));
}

char *appGetText(void)
{
    return config.text;
}

void appSetId(unsigned short id)
{
    config.id_s = id;
    flashWrite((const char *)&config, sizeof(config));
}

unsigned short appGetId(void)
{
    return config.id_s;
}

char *appGetFriendlyName(void)
{
    return config.friendly_name;
}

void appIdToHex(char *buf)
{
    const char hex[] = "0123456789abcdef";
    unsigned short id = appGetId();
    buf[0] = hex[id >> 12];
    buf[1] = hex[(id >> 8) & 0x0f];
    buf[2] = hex[(id >> 4) & 0x0f];
    buf[3] = hex[(id) & 0x0f];
    buf[4] = '\0';
}

unsigned short appHexToId(const char *str)
{
    unsigned short id = 0;
    int i;

    for (i=0; i<4; i++) {
        id <<= 4;
        if (*str >= '0' && *str <= '9') {
            id |= *str - '0';
        } else if (*str >= 'a' && *str <= 'f') {
            id |= *str - 'a' + 10;
        } else {
            return 0xffff;
        }
        str++;
    }

    if (*str)
        return 0xffff;

    return id;
}

void radioPacketReceived(char *buf, char len)
{
    //uartPutString("Got packet data.\n\r");
    P1OUT ^= 2;

    packet_t *pkt = (packet_t *)buf;

    if ((pkt->to[0] || pkt->to[1])
        &&(pkt->to[0] != config.id[1] || pkt->to[1] != config.id[0])) {
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

            unsigned char buf[8];
            appIdToHex(buf);

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
        appSetText(pkt->payload);
        break;

    case CMD_FRIENDLY_NAME:
        if (pkt->size < sizeof(pkt->payload)) {
	        pkt->payload[pkt->size] = 0;
        } else {
	        pkt->payload[sizeof(pkt->payload)-1] = 0;
        }
        appSetFriendlyName(pkt->payload);
        break;

    case CMD_WRITE_ID:
        if (pkt->size != 2) {
	        return;
        }
        appSetId(pkt->payload[1]|(pkt->payload[0] << 8));
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

void appTick(void)
{
    //unsigned short time = flashGetTime();

    if (infection_rate > 0) {
        packet_t pkt = {
            .type = CMD_HEARTBEAT,
            .from = {config.id[0], config.id[1]},
            .payload = {infection_rate, cure_rate}
        };

        calcChecksum((char *)&pkt);
        radioTransmitPacket((char *)&pkt);

        if (cure_rate > (TAR % 100)) {
            infection_rate = 0;
            ledSetState(LED_STATE_SCROLL_TEXT);
        }
    }

    /*static short rtdiv = 0;
    if (rtdiv++ < 60) {
        return;
    }
    rtdiv = 0;

    flashUpdateTime();*/
}

void appInit(void)
{
    infection_rate = cure_rate = 0;

    if (flashRead((char *)&config, sizeof(config))) {
        if (!config.text[0]) {
	        ledSetState(LED_STATE_SCROLL_BARS);
        } else {
	        ledSetText(config.text);
	        ledSetState(LED_STATE_SCROLL_TEXT);
        }
        uartPutString("Initialized.\n\rText: ");
        uartPutString(config.text);
        uartPutString("\n\rFriendly name: ");
        uartPutString(config.friendly_name);
        uartPutString("\n\rID: ");
        char buf[5];
        appIdToHex(buf);
        uartPutString(buf);
        uartPutString("\n\r");
    } else {
        config.id_s = 9999;
        config.friendly_name[0] = '\0';
        ledSetState(LED_STATE_SCROLL_BARS);
        uartPutString("Not initialized.\n\r");
    }
}
