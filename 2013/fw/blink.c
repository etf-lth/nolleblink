#include <msp430.h>

#include "spi.h"
#include "led.h"
#include "uart.h"

/*
 *  P1.0    MCP /CS
 *  P1.7    MCP MOSI
 *  P1.5    MCP SCK
 */

/*__attribute__((interrupt (TIMER0_A0_VECTOR)))
void TIMERA0_ISR(void)
{
    static int div;
    if (div++ == 10)
        div = 0;
    else
        return;

    P2OUT ^= BIT6;
}*/

void uart_received(char c)
{
    char buf[5] = "    ";
    buf[0] = c;
    ledSetText(buf);
    ledSetState(LED_STATE_SCROLL_TEXT);
}

int main(void)
{
    // Disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    
    // Set DCO frequency
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
    BCSCTL2 &= ~(BIT5|BIT4);  // Set MCLK prescaler to 1

#if 0
    // System tick at 100 Hz
    TACCR0 = 20000;
    TACTL = TASSEL_2|MC_2|ID_3; // SMCLK/8?
    TACCTL0 = CCIE;
#endif

    P2DIR |= BIT6;
    P2SEL = 0;
    P2SEL2 = 0;

    spiInit();
    ledInit();
    uart_init();

    //ledSetState(LED_STATE_SCROLL_BARS);

    //ledSetText("yeah!   ");
    //ledSetState(LED_STATE_SCROLL_TEXT);

    ledSetState(LED_STATE_FRAMEBUFFER);

    ledSetBuffer("\0\0\x30\x18\x3d\x56\x5c\x1c\x5c\x56\x3d\x18\x30\0");

    __bis_SR_register((LPM1_bits)|(GIE));
    /*for (;;) {
        P2OUT ^= BIT6;
    }*/
}
