#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include "softu.h"
#include "led.h"

//#define DEBUG

unsigned char bt_version[64], bt_bdaddr[18];

void bt_send(unsigned char type, unsigned char *data, unsigned char len)
{
    softu_transmit(0xa5);
    softu_transmit(type);
    softu_transmit(len);
    while (len--) {
        softu_transmit(*data++);
    }
}

void bt_friendly_name(char *str)
{
    int len;
    char *p;
    for (p=str, len=0; *p; p++, len++) ;
    bt_send(0x04, str, len);
}

void bt_write(char *str)
{
    int len;
    char *p;
    for (p=str, len=0; *p; p++, len++) ;
    bt_send(0x03, str, len);
}

void bt_command(char *str)
{
    switch (str[0]) {
    case 'V':
        switch (str[1]) {
            case '1':
                bt_write("Nolleblinkmojt 2012");
                break;
            case '2':
                bt_write("(c) 2009-2012 ElektroTekniska Föreningen");
                break;
            case '3':
                bt_write(bt_bdaddr);
                break;
            case '4':
                bt_write(bt_version);
                break;
            default:
                goto error;
        }
        break;

    case 'q':
        bt_write("Bye!\n\r");
        bt_send(0x02, NULL, 0);
        return;

    case 't':
        bt_write(appGetText());
        break;

    case 'T':
        appSetText(&str[1]);
        bt_write("OK");
        break;

    case 's':
        if (str[1] < '0' || str[1] > '9')
            goto error;

        if (str[1] == '0') {
            ledSetText(&str[2]);
        } else {
            ledSetTempText(str[1] - '0', &str[2]);
        }
        bt_write("OK");
        break;

    case 'i':
        {
            char buf[5];
            appIdToHex(buf);
            bt_write(buf);
        }
        break;

    case 'I':
        {
            unsigned short id = appHexToId(&str[1]);
            if (id == 0xffff)
                goto error;
            appSetId(id);
            bt_write("OK");
        }
        break;

    case 'z':
        {
            unsigned char buf[8];
            appIdToHex(buf);
            buf[4] = buf[5] = buf[6] = ' ';
            buf[7] = 0;
            ledSetTempText(3, buf);
        }
        break;

    case '\0':
        break;

    default:
        goto error;
    }

    bt_write("\r\n");
    return;

error:
    bt_write("!WTF\n\r");
}

void bt_packet(unsigned char type, unsigned char len, unsigned char *data)
{
    static unsigned char cmdbuf[128], cmdidx = 0;

    switch (type) {
    case 0x00:
        data[len] = '\0';
        strcpy(bt_version, data);

#ifdef DEBUG
        uartPutString("Bluetooth controller booted.\n\rFirmware: ");
        uartPutString(data);
        uartPutString("\r\n");
#endif
        break;

    case 0x01:
        cmdidx = 0;

#ifdef DEBUG
        uartPutString("Connected: ");
        data[len] = '\0';
        uartPutString(data);
        uartPutString("\r\n");
#endif

        bt_write("#nolleblink2012\r\n");
        break;

    case 0x02:
#ifdef DEBUG
        uartPutString("Disconnected.\n\r");
#endif
        break;

    case 0x03:
        {
            while (len--) {
                if (cmdidx == sizeof(cmdbuf)) {
                    cmdidx = 0;
                }

                cmdbuf[cmdidx++] = *data++;
                if (cmdbuf[cmdidx-1] == '\n') {
                    cmdbuf[cmdidx-1] = '\0';
                    bt_command(cmdbuf);
                    cmdidx = 0;
                }
            }
        }
        break;

    case 0x06:
        data[len] = '\0';

#ifdef DEBUG
        uartPutString("BDADDR: ");
        uartPutString(data);
        uartPutString("\r\n");
#endif

        strcpy(bt_bdaddr, data);
        {
            char *f = appGetFriendlyName();
            if (*f) {
                bt_friendly_name(f); 
            }
        }
        break;
    }
}

void bt_process(unsigned char c)
{
    static unsigned char buf[128], idx = 0;

    buf[idx++] = c;

    if (idx == 1 && buf[0] != 0xa5) {
        idx = 0;
        return;
    }

    if (idx >= 3 && buf[2] == idx-3) {
        bt_packet(buf[1], buf[2], &buf[3]);
        idx = 0;
    }
}

