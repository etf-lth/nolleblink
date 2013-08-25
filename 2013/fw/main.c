#include <msp430.h>

#include "spi.h"
#include "led.h"
#include "uart.h"
#include "fifo.h"

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
        uart_puts("Not supported :-(");
        uart_putbrk();
        break;

    case 'b': // Set bitmap
        ledSetState(LED_STATE_FRAMEBUFFER);
        ledSetBuffer(&str[1]);
        break;

    case 't': // Get permanent text
        uart_puts("BlinkText");
        break;

    case 'T': // Set permanent
        ledSetText(&str[1]);
        ledSetState(LED_STATE_SCROLL_TEXT);
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
    static char buf[128], idx = 0;

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
