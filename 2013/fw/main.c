/*
 *  main.c
 *  Nolleblink 2013 | Blinkmojt
 *
 *  Modified by Fredrik Ahlberg on 2013-08-26.
 *  Copyright (c) 2010-2013 ETF. All rights reserved.
 *
 */

#include <msp430.h>
#include <stdlib.h> // for NULL

#include "spi.h"
#include "led.h"
#include "flash.h"
#include "uart.h"
#include "fifo.h"

FIFO_DEF(bt_rx);
volatile int isr_rx, isr_break;

void bt_command(char *str)
{
    switch (str[0]) {
    case 'V': // Get version
        switch (str[1]) {
        case '1':
            uart_puts("Nolleblinkmojt 2013");
            break;

        case '2':
            uart_puts("(c) 2009-2013 ElektroTekniska Föreningen");
            break;

        case '3':
        case '4':
            uart_puts("N/A");
            break;

        default:
            goto error;
        }
        break;

    case 'q': // Disconnect
        uart_puts("Bye!");
        uart_putbrk();
        break;

    case 'b': // Set bitmap
        led_set_state(LED_STATE_FRAMEBUFFER);
        led_set_buffer(&str[1]);
        break;

    case 't': // Get permanent text
        if (flash_read(NULL, 0)) {
            uart_puts((char *)FLASH_MAIN_BASE);
        }
        break;

    case 'T': // Set permanent
        led_set_text(&str[1]);
        led_set_state(LED_STATE_SCROLL_TEXT);
        uart_puts("OK");
        break;

    case 's': // Scroll temporary
        uart_puts("OK");
        break;

    case 'i': // Get ID
        uart_puts("N/A");
        break;

    case 'I': // Set ID
    case 'z': // Scroll ID
        goto error;

    case '\0':
        break;

    default:
        goto error;
    }

    uart_puts("\r\n");
    return;

error:
    uart_puts("!WTF\n\r");
}

void process_rx(char c)
{
    static char buf[128];
    static unsigned char idx;

    if (idx == sizeof(buf)) {
        idx = 0;
    }

    buf[idx++] = c;

    if (buf[idx-1] == '\n') {
        buf[idx-1] = 0;
        bt_command(buf);
        idx = 0;
    }
}

void process_break(void)
{
    uart_puts("#nolleblink2013\r\n");
}

int main(void)
{
    // Disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    
    // Set DCO frequency and calibration
    if (CALBC1_16MHZ != 0xff) {
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_16MHZ;
        DCOCTL = CALDCO_16MHZ;
    } else {
        // The calibration sector has been wasted, use a good guess.
        BCSCTL1 = 0x8f;
        DCOCTL = 0x91;
    }
    BCSCTL2 &= ~(BIT5|BIT4);  // Set MCLK prescaler to 1

    spi_init();
    led_init();
    uart_init();

    while (1) {
        __bis_SR_register((LPM1_bits)|(GIE));

        while (isr_rx | isr_break) {
            if (isr_rx) {
                isr_rx = 0;
                while (!FIFO_EMPTY(bt_rx)) {
                    process_rx(FIFO_GET(bt_rx));
                }
            } else if (isr_break) {
                isr_break = 0;
                process_break();
            }
        }
    }
}
