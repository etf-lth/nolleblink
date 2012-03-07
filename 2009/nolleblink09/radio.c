#include "msp430x22x4.h"
#include "radio.h"
unsigned char readdata[65];
unsigned char curpack[65];
unsigned char lastsent[65];
unsigned int timesincesent;
unsigned int myid;
unsigned int acknr;
unsigned int timeoutpkg;
unsigned int senddelay;
extern unsigned char statusdata;
extern unsigned char sending;
unsigned char fail;
unsigned char resend;
unsigned int lastpkg;
void initradio(unsigned int myidnr){
	
	unsigned char statusdata;
	unsigned char PAtable[8] = {0xff,0,0,0,0,0,0,0};
	myid = myidnr;
	resetradio();
	delay(100);
	writeradiostrobe(0x33);
	statusdata = writeradiostrobe(0x3D);
	while(statusdata == 79){
		delay(10);	
		statusdata = writeradiostrobe(0x3D);
	}
	writeradioreg(0x2,0x6); // write GDO0 to rx interrupt
//	writeradioreg(0x0,0x9); // write GDO2 to CCA pin
	writeradioreg(0x6,60); // 60 byte to send
	writeradioreg(0xa,0x30); // set chan 30
	writeradioreg(0x8,BIT6|BIT2); // set fixed datalength, crc enalbe
	writeradioreg(0x13,BIT7|BIT6|BIT1); // FEC, 8Byte preamble
	writeradioreg(0x12,3|BIT4|BIT5|BIT6); // 16byte sync, MSK modulering
	writeradioreg(0x18,BIT4|BIT2); // autocalibrate when going from idle to rx,tx
	writeradioreg(0x7,BIT3|BIT2|BIT1|BIT5); // crcflush, address and append, preamble quality
	writeradioreg(0x9,150); // address is 150
//	writeradio(0x3e,PAtable,8);
	writeradioreg(0x3e,0xff); // power to the max
}

void setradioid(unsigned int id){
	myid = id;
}
void setreceive(){
	statusdata = writeradiostrobe(0x34); // send set to receive
	while((statusdata >> 4 & 0x7) != 1)
		statusdata = writeradiostrobe(0x3d); // null get status
		

}

void initradioprotocol(){
	TBCCTL0 = CCIE;                           // TACCR0 interrupt enabled
	TBCCR0 = 1000;
	TBCTL = TBSSEL_2 + MC_2 + ID_3;                  // SMCLK, contmode
	TBCTL &= ~MC_2; // stop timer;
	_EINT();
	acknr = 0;
}
// Timer B0 interrupt service routine
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B (void)
{
	timesincesent++;
	if(acknr > 0){
		
		if(timesincesent > timeoutpkg){
			timesincesent = 0;
			sendtxbuffer();	
			resend++;
		}
	}
	if(resend > 5){
		acknr = 0;
		fail = 1;
		stoptimer();
	}
	if(senddelay > 0){
		if(timesincesent > senddelay){
			senddelay = 0;
			sendtxbuffer();
			
		}
	}
		
	TBR = 0; 
}
void starttimer(){
	fail = 0;
	resend = 0;
	timesincesent = 0;
	TBCTL |= MC_2; // start timer;
}
void stoptimer(){
	TBCTL &= ~MC_2; // stop timer;
}
void rxpack(){
	readradio(0x3f,readdata,60);
			statusdata = writeradiostrobe(0x3d);
			writeradiostrobe(0x3a); // flush the bastard
			writeradiostrobe(0x34);	
}
unsigned char validatepack(){
	unsigned char w;
	if((((unsigned int)readdata[1] << 8 | readdata[2]) == myid) || ((unsigned int)readdata[1] << 8 | readdata[2]) == 0 && ((unsigned int)readdata[5] << 8 | readdata[6]) != lastpkg){
		lastpkg = ((unsigned int)readdata[5] << 8 | readdata[6]);
		//if(readdata[63] == 34 && readdata[64] == 88){ // more check for right data
			for(w = 0; w < 60; w++){
				curpack[w] = readdata[w];	
			}
			if(acknr > 0 && curpack[7] == 0){
//				if(((unsigned int)curpack[5] << 8 | curpack[6]) == acknr)
					acknr = 0; // got ack for last package.
					stoptimer();
			}
		//}
		return 1;
	}
	return 0;
}
void txackpack(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size, unsigned int timeout){
	unsigned char q;
	lastsent[0] = 0;
	lastsent[1] = toadr >> 8;
	lastsent[2] = toadr & 0xff;
	lastsent[3] = fromadr >> 8;
	lastsent[4] = fromadr & 0xff;
	lastsent[5] = pktnr >> 8;
	lastsent[6] = pktnr & 0xff;
	lastsent[7] = pkttype;
	lastsent[8] = size;
	for(q = 0; q < size; q++){
		lastsent[9+q] = payload[q];	
	}
	acknr = pktnr;
	timeoutpkg = timeout;
	timesincesent = 0;
	starttimer();
	sendtxbuffer();
}

void txpack(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size ){
	unsigned char q;
	
	lastsent[0] = 0;
	lastsent[1] = toadr >> 8;
	lastsent[2] = toadr & 0xff;
	lastsent[3] = fromadr >> 8;
	lastsent[4] = fromadr & 0xff;
	lastsent[5] = pktnr >> 8;
	lastsent[6] = pktnr & 0xff;
	lastsent[7] = pkttype;
	lastsent[8] = size;
	for(q = 0; q < size; q++){
		lastsent[9+q] = payload[q];	
	}
	sendtxbuffer();
}
void txpackquick(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size ){
	unsigned char q;
	
	lastsent[0] = 0;
	lastsent[1] = toadr >> 8;
	lastsent[2] = toadr & 0xff;
	lastsent[3] = fromadr >> 8;
	lastsent[4] = fromadr & 0xff;
	lastsent[5] = pktnr >> 8;
	lastsent[6] = pktnr & 0xff;
	lastsent[7] = pkttype;
	lastsent[8] = size;
	sendtxbuffer();
	
}
void txpackdelay(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size, unsigned int txdelay ){
	unsigned char q;
	senddelay = txdelay;
	acknr = 0;
	lastsent[0] = 0;
	lastsent[1] = toadr >> 8;
	lastsent[2] = toadr & 0xff;
	lastsent[3] = fromadr >> 8;
	lastsent[4] = fromadr & 0xff;
	lastsent[5] = pktnr >> 8;
	lastsent[6] = pktnr & 0xff;
	lastsent[7] = pkttype;
	lastsent[8] = size;
	for(q = 0; q < size; q++){
		lastsent[9+q] = payload[q];	
	}
	starttimer();
//	sendtxbuffer();
}
void sendtxbuffer(){
	sending = 1;
	lastsent[58] = 0x33;
	lastsent[59] = 0x45;
	while(P2IN & BIT6){
//		senddelay = 100;
//		starttimer();
//		delay(1000);
	}
//	else{
		senddelay = 0;
		P2IE &= ~BIT6;                             // P1.2 interrupt enabled
		writeradio(0x7f,lastsent,60);
		writeradiostrobe(0x35);
//		statusdata = writeradiostrobe(0x3d);
//		while(!(P2IN & BIT6));
		delay(10000);
		while(P2IN & BIT6);
		
//		if(acknr == 0)
			stoptimer();
			
		writeradiostrobe(0x36); // goidle
		writeradiostrobe(0x3a); // flush the bastard
		setreceive();
		sending = 0;
		
		P2IFG &= ~BIT6;                           // P1.2 IFG cleared
		P1OUT ^= BIT1;
		P2IE |= BIT6;                             // P1.2 interrupt enabled
		
//	}
}

void resetradio(){
	P3OUT &= ~BIT0; // 
	P3OUT |= BIT0; // cs high
	delay(1000);
	P3OUT &= ~BIT0; // 
	while(P3IN & BIT2);
	sendspi(0x30);
	while(P3IN & BIT2);
	P3OUT |= BIT0; // cs high
}

void initradiospi(){
	P3SEL |= 0x0E;                            // P3.3,2 USCI_B0 option select
	P3DIR |= BIT0;
	P3OUT |= BIT0;
	UCB0CTL0 |= UCMSB + UCMST + UCSYNC + UCCKPH;       // 3-pin, 8-bit SPI mstr, MSB 1st
  	UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  	UCB0BR0 = 0x10;
  	UCB0BR1 = 0;
  	UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  	
  	
  	
  	
  	
}
unsigned char sendspi(unsigned char data){
	static unsigned char rec;
    UCB0TXBUF = data;                       // Dummy write to start SPI
    while (!(IFG2 & UCB0RXIFG));            // USCI_B0 RX buffer ready?
    rec = UCB0RXBUF;                       // data = 00|DATA
	return rec;
}

unsigned char writeradiostrobe(unsigned char strobe){
	static unsigned char status;
	status = 0;
	P4OUT |= (1 << 6);   
	P3OUT &= ~BIT0; // cs low
	while(P3IN & BIT2);
	status = sendspi(strobe);
	P3OUT |= BIT0; // cs high
	return status;	
	
}
unsigned char writeradioreg(unsigned char adr,unsigned char data){
	static unsigned char status;
	status = 0;
	P4OUT |= (1 << 6);   
	P3OUT &= ~BIT0; // cs low
	while(P3IN & BIT2);
	status = sendspi(adr);
	status = sendspi(data);
	P3OUT |= BIT0; // cs high
	return status;	
	
}

unsigned char writeradio(unsigned int adr,unsigned char *data,unsigned int length){
	unsigned char status;
	unsigned char q;
	P4OUT |= (1 << 6);   
	P3OUT &= ~BIT0; // cs low
	while(P3IN & BIT2);
	status = sendspi(BIT6|adr); // burst mode adr;
	
	for(q = 0; q < length; q++){
		status = sendspi(data[q]);
		if((status & 0xf) == 1)
			break;
	}
	P3OUT |= BIT0; // cs high
	return status;	
	
}
unsigned char readradio(unsigned int adr,unsigned char *data,unsigned int length){
	unsigned char status;
	unsigned char q;
	P4OUT |= (1 << 6);   
	P3OUT &= ~BIT0; // cs low
	while(P3IN & BIT2);
	status = sendspi(BIT7|BIT6|adr); // burst mode adr;
	
	for(q = 0; q < length; q++){
		data[q] = sendspi(0xa);
	}
	P3OUT |= BIT0; // cs high
	return status;	
	
}

unsigned char readradioregisters(unsigned char adr){
	static unsigned char status;
	status = 0;
	P4OUT |= (1 << 6);   
	P3OUT &= ~BIT0; // cs low
	while(P3IN & BIT2);
	status = sendspi(BIT7|adr);
	status = sendspi(0); // dummy write to clock out data
	P3OUT |= BIT0; // cs high
	return status;	
	
}
