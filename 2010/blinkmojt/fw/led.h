/*
 *  led.h
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#ifndef LED_H
#define LED_H

enum {
  LED_STATE_OFF,
  LED_STATE_SCROLL_TEXT,
  LED_STATE_SCROLL_TEMP,
  LED_STATE_SCROLL_BARS,
  LED_STATE_FRAMEBUFFER,
  LED_STATE_VIRUS
};

void ledInit();
void ledSetState(unsigned int);
void ledSetText(const char *);
void ledSetTempText(char, const char *);
void ledSetBuffer(const char *);

#endif
