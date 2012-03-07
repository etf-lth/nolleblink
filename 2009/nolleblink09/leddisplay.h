#ifndef LEDDISPLAY_H_
#define LEDDISPLAY_H_

#endif /*LEDDISPLAY_H_*/
#include "msp430x22x4.h"
void initleds();
int sendspi(unsigned char data);
void initledspi();
void sendcommand(unsigned char reg, unsigned char data);
void delay(unsigned int delay);
void initleddisp();
void setleds(unsigned int leds);
void pushnext();
void pushcol(unsigned char col);
void settext(char *text1);
void empty();
void sync();
#include "ledmatrixascii.h"
#define SCROLLSPEED 40
volatile unsigned int i;
volatile unsigned char ledarray[15];
unsigned int row;
unsigned int selfcontrolled;
unsigned char text[42];
unsigned char length;
unsigned char col;
unsigned int speed;
unsigned char letter;
int g;