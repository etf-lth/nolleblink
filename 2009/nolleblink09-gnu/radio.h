#ifndef RADIO_H_
#define RADIO_H_
void resetradio();
unsigned char sendspi(unsigned char data);
unsigned char writeradiostrobe(unsigned char strobe);
unsigned char writeradio(unsigned int adr,unsigned char *data,unsigned int length);
unsigned char readradio(unsigned int adr,unsigned char *data,unsigned int length);
unsigned char readradioregisters(unsigned char adr);
unsigned char writeradioreg(unsigned char adr,unsigned char data);
void txackpack(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size,
 unsigned int timeout);
 void txpack(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size );
void txpackdelay(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size, unsigned int txdelay );
void setreceive();
unsigned char validatepack();
void initradio();
void initradiospi();
void rxpack();
void sendtxbuffer();
void stoptimer();
void starttimer();
void setradioid(unsigned int id);
void txpackquick(unsigned int toadr, unsigned int fromadr, unsigned int pktnr, unsigned char pkttype, unsigned char *payload,unsigned char size );

#endif /*RADIO_H_*/
