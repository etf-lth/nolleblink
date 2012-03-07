#include "leddisplay.h"

extern unsigned char gotpacket;
extern unsigned char sending;
extern unsigned char readflash();
extern unsigned char readdata[65];
extern unsigned int myid;
static unsigned char siffra[3];
unsigned char tosend;
unsigned char lastcol;
unsigned char state;
unsigned int scrlnbr;
unsigned char times;
unsigned char d;
void initleddisp(){
	empty();
	//---------- init spi-------------------
	P4DIR |= BIT6 | BIT4 | BIT3;
	P2DIR |= BIT0|BIT1|BIT2|BIT3|BIT4;
	P2OUT = 0;
	P4OUT = 1 << 6;
	P3DIR |= BIT0;
	P3OUT |= BIT0;
	for(g = 0; g < 15; g++){
		pushcol(0);
	}
	initledspi();
	//--------------------------------------
	//---------init timer-------------------
	TACCTL0 = CCIE;                           // TACCR0 interrupt enabled
	TACCR0 = 20000;
	TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode
	_EINT();
	  
	//----------------------------------------
	selfcontrolled = 1;
	 
	length = 0;
	row = 0;
	letter = 0;
	col = 0;
	initleds();
		
	for(g = 0; g < 15; g++){
		pushcol(0);
	}
}

void initledspi(){
	UCB0CTL0 |= UCMSB + UCMST + UCSYNC + UCCKPH;       // 3-pin, 8-bit SPI mstr, MSB 1st
	UCB0CTL1 |= UCSSEL_2;                     // SMCLK
	UCB0BR0 = 0x3;								// divide by 16
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}
void setstate(unsigned char instate){
	if(instate == 0){
		// gettext(); // get text from flash
		readflash();
		state = 0;	
	}
	else if(instate == 1){
		scrlnbr = 0;
		if(times < 2){
			sync();	
		}
		state = 1;
	}
	else if(instate == 2){
		for(g = 0; g < 15; g++){// empty screen
			pushcol(0);
		}
		state = 2;
	}
	else if(instate == 3){ // show idnummer
		
	scrlnbr = 0;
		
		for(g = 0; g < 15; g++){// empty screen
			pushcol(0);
		}
		state = 3;
		siffra[0] = myid/100;
		siffra[1] = (myid - siffra[0]*100)/10;
		siffra[2] = myid - siffra[0]*100 - siffra[1]*10;
		for(g = 0; g < 3; g++){
			for(d = 0; d < 5; d++){
					pushcol(alfatable[(siffra[g]+48)*5+d]);
			}
		}
	}
	else if(instate == 4){ // virus
		state = 4;
		for(g = 0; g < 15; g++){// empty screen
			pushcol(0);
		}
	
		siffra[0] = ' ';
		siffra[1] = 10;
		siffra[2] = ' ';
		for(g = 0; g < 3; g++){
			for(d = 0; d < 5; d++){
					pushcol(alfatable[(siffra[g])*5+d]);
			}
		}
	}

	
	
}

void sync(){
	
	letter = 0;
	col = 0;
	row = 0;
	for(g = 0; g < 15; g++){
		pushcol(0);
	}
}

void empty(){
	static int g;
	for(g = 0; g < 42; g++)
		text[g] = 0;
	for(g = 0; g < 14; g++)
		ledarray[i] = 0;	
	
}
void settext(char *text1){
	length = 0;
	while(*text1){
		text[length] = *text1++;
		length++;
	}
	
}


//
// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	static unsigned char last = 0;
//	if(sending == 0){
//		if(P2IN & BIT6)
//			last = 1;
//		if((last == 1) && !(P2IN & BIT6)){
//			last = 0;
//		
//			gotpacket = 1;
//		}
//	}
//	else{
//		if(sending > 1)
//			sending = 0;
//		last = 0;
//		gotpacket = 0;	
//	}
	speed++;
	pushnext();
	if(state  == 0){
		if(speed > SCROLLSPEED && length > 0){
//			P1OUT ^= 1;
				speed = 0;
				if(col < 5){
					if(text[letter] == 3)
						text[letter] = 'å';
					else if(text[letter] == 4)
						text[letter] = 'ä';
					else if(text[letter] == 5)
						text[letter] = 'ö';
					else if(text[letter] == 6)
						text[letter] = 'Å';
					else if(text[letter] == 7)
						text[letter] = 'Ä';
					else if(text[letter] == 8)
						text[letter] = 'Ö';
					pushcol(alfatable[text[letter]*5+col]);
					col++;
				}
				else{
					pushcol(0);
					col = 0;
					if(letter < length-1)
						letter++;
					else
						letter = 0;	
				}
		}
	}
	else if(state == 1){// scroll text x times then go back to flashtext
		if(speed > SCROLLSPEED && length > 0 && scrlnbr < times){
//			P1OUT ^= 1;
				speed = 0;
				if(col < 5){
					if(text[letter] == 3)
						text[letter] = 'å';
					else if(text[letter] == 4)
						text[letter] = 'ä';
					else if(text[letter] == 5)
						text[letter] = 'ö';
					else if(text[letter] == 6)
						text[letter] = 'Å';
					else if(text[letter] == 7)
						text[letter] = 'Ä';
					else if(text[letter] == 8)
						text[letter] = 'Ö';
					pushcol(alfatable[text[letter]*5+col]);
					col++;
					
				}
				else{
					pushcol(0);
					col = 0;
					if(letter < length-1)
						letter++;
					else{
						letter = 0;	
						scrlnbr++;
					}
				}
		}
		else if(scrlnbr >= times){
			setstate(0); // set flashtext again	
		}
	}
	else if(state == 2){ 
	
	}
	else if(state == 3){ // show idnummer
		speed = 0;	
		scrlnbr++;
		if(scrlnbr > 1000)
			setstate(0);
	}
	else if(state == 4){
		
	}
	
  	TAR = 0;                          // reset
}

void pushcol(unsigned char col){
	static int g;
	for(g = 14; g > 0; g--){
		ledarray[g] = ledarray[g-1];
	}
	ledarray[0] = col;

}
void pushnext(){
	static unsigned int q;
	static unsigned int leds;
	leds = 0;
	for(q = 0; q < 15; q++){
		if(ledarray[q] & (1 << row))
			leds |= 1 << q;	
	}
	P4OUT &= ~(BIT3|BIT4);
	P2OUT = 0;
	setleds(leds);
	
	if(row < 5){
		P2OUT = 1 << row;
	}
	else{
		P4OUT |= 1 << (row - 2);
	}
	row++;
	if(row > 6)
		 row = 0;
	
}

void initleds(){
	P3OUT |= BIT0; // cs high
	P4OUT &= ~(1 << 6);                         // set cs low
	sendspi(BIT6); // address
	sendspi(0);
	sendspi(0);
	sendspi(0);
	P4OUT |= (1 << 6);                         // set cs low
}
void setleds(unsigned int leds){
	P3OUT |= BIT0; // cs high
	P4OUT &= ~(1 << 6);                         // set cs low
	sendspi(BIT6); // address
	sendspi(0x12);
	sendspi(leds & 0xff);
	sendspi(leds >> 8);
	P4OUT |= (1 << 6);                         // set cs low
}

//int sendspi(unsigned char data){
//	static char rec;
//    UCB0TXBUF = data;                       // Dummy write to start SPI
//    while (!(IFG2 & UCB0RXIFG));            // USCI_B0 RX buffer ready?
//    rec = UCB0RXBUF;                       // data = 00|DATA
//	return rec;
//}

void sendcommand(unsigned char reg, unsigned char data){
	P3OUT |= BIT0; // cs high
	P4OUT &= ~(1 << 6);                         // set cs low
	sendspi(BIT6); // address
	sendspi(reg);
	sendspi(data);
	P4OUT |= (1 << 6);                         // set cs low
}
