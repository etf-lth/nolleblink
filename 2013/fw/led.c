/*
 *  led.c
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Modified by Fredrik Ahlberg on 2010-06-24.
 *  Original implementation by Joakim Arnsby.
 *  Copyright (c) 2009 - 2010 ETF. All rights reserved.
 *
 */

#include <msp430.h>
#include <string.h>

#include "led.h"
#include "spi.h"
#include "board.h"
#include "flash.h"
#include "ledfont.h"

/* private functions */
static void led_update_frame(void);
static void led_update_row(void);
static void led_init_hw(void);
static void led_set_row(unsigned int);

static unsigned char ledarray[15];
static unsigned int row, state, scrolldiv, scrollcol, scrollchar, fbtimeout;

const unsigned char bars[4] = {0x19, 0x33, 0x66, 0x4C};

static unsigned char textbuf[127], tempcount;

static void led_default_state(void)
{
    if (flash_read(textbuf, sizeof(textbuf)-1) && textbuf[0]) {
        textbuf[sizeof(textbuf)-1] = '\0';
        led_set_state(LED_STATE_SCROLL_TEXT);
    } else {
        led_set_state(LED_STATE_SCROLL_BARS);
    }
}

void led_init(void)
{
    // Row drivers + LED /CS as outputs
    LED_ROW_PxSEL = 0;
    LED_ROW_PxSEL2 = 0;
    LED_ROW_PxDIR |= LED_ROW_PINS;
    LED_CSn_PxDIR |= LED_CSn_PIN;
    LED_CSn_PxOUT |= LED_CSn_PIN;
    LED_ROW_PxOUT = 0;

    // Init IO expander
    led_init_hw();

    // Init LED multiplex timer
    TACCR0 = 10000;
    TACTL = TASSEL_2|MC_2;
    TACCTL0 = CCIE;

    // Try load state from flash
    led_default_state();
}

void led_set_state(unsigned int new)
{
    state = new;
    scrolldiv = scrollcol = scrollchar = 0;
    if (new == LED_STATE_FRAMEBUFFER) {
        fbtimeout = 50;
    } else {
        memset(ledarray, 0, sizeof(ledarray));
    }

    /*if (new == LED_STATE_VIRUS) {
        int idx;
        char virus[] = {32, 10, 32};
        for (idx=0; idx<15; idx++) {
	        ledarray[idx] = smallfont[virus[idx / 5]*5+(idx%5)];
        }
    }*/

    if (new == LED_STATE_SCROLL_TEXT && !strcmp(textbuf, "invaders")) {
        led_set_state(LED_STATE_FRAMEBUFFER);
        led_set_buffer("\0\0\x30\x18\x3d\x56\x5c\x1c\x5c\x56\x3d\x18\x30\0");
        fbtimeout = -1; // forever-isch
    }
}

void led_set_text(const char *text)
{
    strncpy(textbuf, text, sizeof(textbuf));
    flash_write(textbuf, sizeof(textbuf)-1);
}

void led_set_temp_text(char count, const char *text)
{
    if (state != LED_STATE_SCROLL_TEMP || strncmp(text, textbuf, sizeof(textbuf))) {
        led_set_state(LED_STATE_SCROLL_TEMP);
        strncpy(textbuf, text, sizeof(textbuf));
        tempcount = count;
    }
}

void led_set_buffer(const char *buf)
{
    memcpy(ledarray, buf, sizeof(ledarray));
}

static void led_update_frame(void)
{
    unsigned int idx;
    static int div;

    switch (state) {
    case LED_STATE_SCROLL_TEMP:
    case LED_STATE_SCROLL_TEXT:
        for (idx = 0; idx < 14; idx++) {
	        ledarray[idx] = ledarray[idx+1];
        }

        if (scrollcol == 5) {
	        ledarray[14] = 0;
        } else {
	        ledarray[14] = smallfont[textbuf[scrollchar]*5+scrollcol];
        }

        scrollcol++;
        if (scrollcol == 6) {
	        if (!textbuf[++scrollchar]) {
	            scrollchar = 0;
	            if (state == LED_STATE_SCROLL_TEMP && !--tempcount) {
                    led_default_state();
                }
            }
	        scrollcol = 0;
	    }
        break;

    case LED_STATE_SCROLL_BARS:
        div = !div;
        if (div) {
            for (idx = 0; idx < 15; idx++) {
                ledarray[idx] = bars[(idx+scrollcol) & 0x03];
            }
    
            scrollcol++;
            if (scrollcol == 4) {
                scrollcol = 0;
            }
        }
        break;

    case LED_STATE_FRAMEBUFFER:
        if (--fbtimeout == 0) {
            led_default_state();
	    }
        break;
    }
}

static void led_init_hw(void)
{
    LED_CSn_PxOUT &= ~LED_CSn_PIN;  // led cs low

    spi_xfer(BIT6);                 // command byte: write, slave address 0
    spi_xfer(0);                    // register IODIRA
    spi_xfer(0);
    spi_xfer(0);

    LED_CSn_PxOUT |= LED_CSn_PIN;   // led cs high
}

static void led_set_row(unsigned int leds)
{
    LED_CSn_PxOUT &= ~LED_CSn_PIN;  // led cs low

    spi_xfer(BIT6);                 // command byte: write, slave address 0
    spi_xfer(0x12);                 // register GPIOA
    spi_xfer(leds & 0xff);
    spi_xfer(leds >> 8);

    LED_CSn_PxOUT |= LED_CSn_PIN;   // led cs high
}

static void led_update_row(void)
{
    static int div;

    if (++div == 2) {
        LED_ROW_PxOUT = 0;
        div = 0;
    } else {
        unsigned int col, leds, mask;

        if (state == LED_STATE_OFF) {
            // disable row
            P2OUT = 0;
            return;
        }

        // assemble column pixels
        leds = 0;
        mask = 1 << row;
        for (col=0; col<15; col++) {
            if (ledarray[col] & mask) {
                leds |= 0x4000 >> col;
            }
        }

        // disable row
        LED_ROW_PxOUT = 0;

        // update column drivers
        led_set_row(leds);
      
        // enable row
        LED_ROW_PxOUT = 0x40 >> row;

        // update row counter
        row++;
        if (row > 6) {
            row = 0;
        }
    }
}

__attribute__((interrupt (TIMER0_A0_VECTOR)))
void led_timer_ISR(void)
{
    if (++scrolldiv == 80) {
        led_update_frame();
        scrolldiv = 0;
    }

    led_update_row();
    TAR = 0; 
}
