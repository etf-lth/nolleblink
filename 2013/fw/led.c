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
#include "ledfont.h"

/* private functions */
static void ledUpdateFrame(void);
static void ledUpdateRow(void);
static void ledInitHW(void);
static void ledSetRow(unsigned int);

unsigned char ledarray[15];
unsigned int row, state, scrolldiv, scrollcol, scrollchar, fbtimeout;

const char bars[4] = {0x19, 0x33, 0x66, 0x4C};

unsigned char textbuf[40], tempbuf[40], tempcount;

void ledInit(void)
{
    // Row drivers + LED /CS
    P2DIR |= BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6;
    LED_CSn_PxOUT |= LED_CSn_PIN;

    P2OUT = 0;

    // Init LED multiplex timer
    TACCTL0 = CCIE;
    TACCR0 = 40000;
    TACTL = TASSEL_2|MC_2;

    ledInitHW();
}

void ledSetState(unsigned int new)
{
    state = new;
    scrolldiv = scrollcol = scrollchar = 0;
    if (new == LED_STATE_FRAMEBUFFER) {
        fbtimeout = 0;
    }

    memset(ledarray, 0, 15);

    if (new == LED_STATE_VIRUS) {
        int idx;
        char virus[] = {32, 10, 32};
        for (idx=0; idx<15; idx++) {
	        ledarray[idx] = smallfont[virus[idx / 5]*5+(idx%5)];
        }
    }
}

void ledSetText(const char *text)
{
    strncpy(textbuf, text, sizeof(textbuf));
}

void ledSetTempText(char count, const char *text)
{
    if (state != LED_STATE_SCROLL_TEMP || strncmp(text, tempbuf, sizeof(tempbuf))) {
        ledSetState(LED_STATE_SCROLL_TEMP);
        strncpy(tempbuf, text, sizeof(tempbuf));
        tempcount = count;
    }
}

void ledSetBuffer(const char *buf)
{
    memcpy(ledarray, buf, 15);
}

static void ledUpdateFrame(void)
{
    unsigned int idx;

    switch (state) {
    case LED_STATE_SCROLL_TEMP:
        for (idx = 0; idx < 14; idx++) {
	        ledarray[idx] = ledarray[idx+1];
        }

        if (scrollcol == 5) {
            ledarray[14] = 0;
        } else {
            ledarray[14] = smallfont[tempbuf[scrollchar]*5+scrollcol];
        }

        scrollcol++;
        if (scrollcol == 6) {
            if (!tempbuf[++scrollchar]) {
	            scrollchar = 0;
	            if (!--tempcount) {
		            ledSetState(LED_STATE_SCROLL_TEXT);
                }
	        }
	        scrollcol = 0;
	    }
        break;      

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
            }
	        scrollcol = 0;
	    }
        break;

    case LED_STATE_SCROLL_BARS:
        {
            static int div = 0;
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
        }
        break;

    /*case LED_STATE_FRAMEBUFFER:
      if(fbtimeout++ == 50)
	{
	  memset(ledarray, 0, 15);
	  ledSetState(LED_STATE_SCROLL_TEXT);
	}
      break;*/
    }
}

__attribute__((interrupt (TIMER0_A0_VECTOR)))
void ledTimerISR(void)
{
    if (++scrolldiv == 20) {
        ledUpdateFrame();
        scrolldiv = 0;
    }

    ledUpdateRow();
    TAR = 0; 
}

static void ledUpdateRow(void)
{
    unsigned int col, leds, mask;

    if (state == LED_STATE_OFF) {
        // disable row
        //P4OUT &= ~(BIT3|BIT4);
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
    //P4OUT &= ~(BIT3|BIT4);
    P2OUT = 0;

    // update column drivers
    ledSetRow(leds);
  
    // enable row
    //P2OUT = 1 << row;
    P2OUT = 0x40 >> row;

    // update row counter
    row++;
    if (row > 6) {
        row = 0;
    }
}

static void ledInitHW(void)
{
    LED_CSn_PxOUT &= ~LED_CSn_PIN;     // led cs low

    spiTransferFast(BIT6);                 // command byte: write, slave address 0
    spiTransferFast(0);                    // register IODIRA
    spiTransferFast(0);
    spiTransferFast(0);

    LED_CSn_PxOUT |= LED_CSn_PIN;      // led cs high
}

static void ledSetRow(unsigned int leds)
{
    LED_CSn_PxOUT &= ~LED_CSn_PIN;     // led cs low

    spiTransferFast(BIT6);                 // command byte: write, slave address 0
    spiTransferFast(0x12);                 // register GPIOA
    spiTransferFast(leds & 0xff);
    spiTransferFast(leds >> 8);

    LED_CSn_PxOUT |= LED_CSn_PIN;      // led cs high
}
