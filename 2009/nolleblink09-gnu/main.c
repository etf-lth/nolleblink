//******************************************************************************
//  MSP430x2xx Nolleblinkmojt 2009
//  By: Joakim Arnsby
//  Built with CCE for MSP430 Version: 3.0
//******************************************************************************

#include <io.h>
#include <signal.h>

#include "msp430x22x4.h"

unsigned int myadr;
void delay();
void readpack();
unsigned char readflash();
void writeflash(char value);
unsigned char statusdata;
extern unsigned char text[42];
extern unsigned char readdata[65];
extern unsigned char length;
extern unsigned char times;
unsigned char virus[3];
//unsigned char test[800];
unsigned char gotpacket;
unsigned char sending;
unsigned int pkt;
unsigned int lastpkt;
unsigned int q;
extern unsigned char tosend;
extern unsigned char lastcol;
extern unsigned char state;
unsigned int timer1;
unsigned int timer2;
unsigned int sendings = 50;

int main(void)
{
  pkt = 1;
  gotpacket = 0;
  sending = 0;
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  BCSCTL1 = CALBC1_16MHZ;                       // Set DCO to 16MHz
  DCOCTL = CALDCO_16MHZ;
  BCSCTL2 &= ~(BIT5|BIT4);
  delay(1000);
  P1DIR |= 3;
  P3SEL |= 0x0E;                            // P3.3,2 USCI_B0 option select
  P3DIR |= BIT0;
  P3OUT |= BIT0;
  initleddisp();

  if(readflash() == 0)
    {
      myadr = 10;
      settext("Gammal och Dryg!!      ");
    }

  setstate(0); // scroll text
  virus[0] = 1;
  virus[1] = 1;
  virus[2] = 1;
  initradio(myadr);
  delay(100);
  // interruptmoj
  P2SEL = 0;
  P2DIR &= ~(BIT6|BIT7);
  
  P2IES |= BIT6;                            // P1.2 Hi/lo edge
  P2IFG &= ~BIT6;                           // P1.2 IFG cleared
  __bis_SR_register(GIE);
  setreceive();
  initradioprotocol();
  P2IE |= BIT6;                             // P1.2 interrupt enabled
  tosend = 0;
  timer1 = 0;
  timer2 = 0;
	
  for (;;)
    {
      if(gotpacket)
	{
	  P1OUT ^= BIT0;
	  gotpacket = 0;
	  
	  TACTL &= ~MC_2; // stop timer;
	  rxpack();
	  
	  //		
	  if(validatepack())
	    readpack();
	  
	  TACTL |= MC_2; // start timer
	}

      if(state == 4)
	{// got virus
	  timer1++;

	  if(timer1 > 65000)
	    {
	      timer2++;	
	      timer1 = 0;
	    }
	
	  if(timer2 > sendings)
	    {
	      timer1 = 0;
	      timer2 = 0;

	      if((TAR % 100) <= virus[1])
		setstate(0);
	      
	      txpack(0, myadr,pkt,9,&virus,3);
	      pkt++;
	      sendings = 25 + (TAR % 75);
	    }  
	}
    }
}

void readpack()
{
  unsigned char i;

  if(readdata[7] == 1)
    { // retrn a pong
      sending = 1;
      P2IE &= ~BIT6;                             // P1.2 interrupt enabled
      delay(6000);

      for(q = 0; q < (TAR % 50)*10; q++)
	{
	  delay(1000);
	  pushnext();
	}

      while(P2IN & BIT6)
	{
	  for(q = 0; q < (TAR % 10)*10; q++)
	    {
	      delay(2000);
	    }
	}

      sending = 1;
		
      txpack((unsigned int)readdata[3]<<8|readdata[4], myadr,pkt,5,&readdata);
      
      pkt++;
      P2IFG &= ~BIT6;                           // P1.2 IFG cleared
      P2IE |= BIT6;                             // P1.2 interrupt enabled
    }
  else if(readdata[7] == 2)
    { // got a textmessage OBS IMPLEMENT MULTIPLE TIMES SCROLLING...		
      length = readdata[8];
      times = readdata[9];

      for(i = 0; i < length; i++)
	{
	  text[i] = readdata[10+i];	
	}

      text[length] = ' ';
      length--;
      sending = 1;
      setstate(1); // sets scrol this text x times, then return to flashtext
      delay(1000);
      
      txpack((unsigned int)readdata[3]<<8|readdata[4],
	     myadr,readdata[5]<<8|readdata[6],0,&readdata,0);
		
      sending = 0;
    }
  else if(readdata[7] == 3)
    { // show id nbr
      setstate(3);
    }
  else if(readdata[7] == 4)
    { // sync
      sync();
      txpack((unsigned int)readdata[3]<<8|readdata[4], myadr,
	     readdata[5]<<8|readdata[6],0,&readdata,0);
      //		setreceive();
    }
  else if(readdata[7] == 5)
    { // pong
    }
  else if(readdata[7] == 6)
    {
    }
  else if(readdata[7] == 7)
    { // write text to flash
      writeflash(0);
      readflash();
      txpack((unsigned int)readdata[3]<<8|readdata[4], myadr,
	     readdata[5]<<8|readdata[6],0,&readdata,0);
    }
  else if(readdata[7] == 8)
    { // writes text to flash and writes id from readdata[9]
      writeflash(1);
      readflash();
      txpack((unsigned int)readdata[3]<<8|readdata[4], myadr,
	     readdata[5]<<8|readdata[6],0,&readdata,0);
		
    }
  else if(readdata[7] == 9)
    {
      if((TAR % 100) <= readdata[9] || readdata[11] == 33)
	{ //got virus
	  setstate(4); // set virusstate
	  virus[0] = readdata[9]; // infection rate
	  virus[1] = readdata[10]; // healing rate
	}	
    }
}

//#pragma vector=PORT2_VECTOR
//__interrupt void Port_2(void)
//{
//	gotpacket = 1;
//  P1OUT ^= 0x01;                            // P1.0 = toggle
//  P2IFG &= ~BIT6;                           // P1.2 IFG cleared
//}

unsigned char readflash()
{
  char *Flash_ptr;
  int i;

  Flash_ptr = (char *)0x1040; 
  if(Flash_ptr[0] == 0x33 && Flash_ptr[1] == 0x44
     && Flash_ptr[2] == 0x55 && Flash_ptr[3] == 0x66)
    { // valid data in flash
      myadr = Flash_ptr[4];
      setradioid(myadr);
      length = Flash_ptr[5];

      for(i = 0; i < length+1; i++)
	{
	  text[i] = Flash_ptr[6+i];	
	}

      return 1;	
    }

  return 0;	
}

void writeflash(char value)
{
  char *Flash_ptr;                          // Flash pointer
  unsigned int i;
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1MHz
  DCOCTL = CALDCO_1MHZ;
  FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator
  Flash_ptr = (char *)0x1040;               // Initialize Flash pointer
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  *Flash_ptr = 0;                           // Dummy write to erase Flash seg

  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
	
  for (i = 0; i < 64; i++)
    {
      if(i == 0)
	{
	  *Flash_ptr++ = 0x33;
  	}
      else if(i == 1)
	*Flash_ptr++ = 0x44;
      else if(i == 2)
	*Flash_ptr++ = 0x55;
      else if(i == 3)
	*Flash_ptr++ = 0x66;
      else if(i == 4)
	{
	  if(value == 1)
	    { // write id from pkt
	      *Flash_ptr++ = readdata[9];
	    }
	  else
	    {
	      *Flash_ptr++ = myadr;	
	      setradioid(myadr);
	    }
  	}
      else if(i == 5)
	{
	  *Flash_ptr++ = readdata[8]-1;
  	}
      else if(i > 5 && i < readdata[8]+6)
	{
	  *Flash_ptr++ = readdata[4+i];
  	}    
      else
	*Flash_ptr++ = 'a';          
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  BCSCTL1 = CALBC1_16MHZ;                    // Set DCO to 16MHz
  DCOCTL = CALDCO_16MHZ;
  BCSCTL2 &= ~(BIT5|BIT4);
}

//#pragma vector=PORT2_VECTOR
//__interrupt void Port_2(void)
interrupt (PORT2_VECTOR) port2_isr(void)
{
  if(sending == 0)
    {
      gotpacket = 1;
      //  P1OUT ^= 0x02;                            // P1.0 = toggle
      P1OUT ^= 1;
    }

  P2IFG &= ~BIT6;                           // P1.2 IFG cleared
}

void delay(unsigned int delay)
{
  do (delay--);
  while (delay != 0);	
}
