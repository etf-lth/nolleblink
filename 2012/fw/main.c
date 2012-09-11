/*
 *  main.c
 *  Nolleblink 2012 | Blinkmojt
 *
 *  Created by Fredrik Ahlberg on 2010-06-24.
 *  Copyright (c) 2010-2012 ETF. All rights reserved.
 *
 */

#include <msp430.h>
#include <string.h>

#include "spi.h"
#include "led.h"
#include "radio.h"
#include "uart.h"
#include "flash.h"
#include "protocol.h"
#include "board.h"
#include "fifo.h"
#include "softu.h"

volatile unsigned char isr_radio, isr_timer, isr_uart;

FIFO_EXTERN(rx_fifo);

//void uartByteReceived(char c)
//{
  /*packet_t pkt = {
    .to = {0, 0},
    .from = {0, 0},
    .type = CMD_WRITE_ID,
    .size = 2,
    .payload = {0x27, 0x0f} //2, 'l', 'u', 'l', 'z', 0, 0xff, 0xaa, 0x55}
  };

  radioTransmitPacket((char *)&pkt);*/

  //uartPutString("Nolleblink 2010 ");
  //}

__attribute__((interrupt (PORT2_VECTOR)))
void PORT2_ISR(void)
{
    softu_interrupt();

    if (RADIO_GDO0_PxIFG & RADIO_GDO0_PIN) {
        isr_radio = 1;
        RADIO_GDO0_PxIFG &= ~RADIO_GDO0_PIN;
        __bic_SR_register_on_exit(LPM1_bits);
    }
}

asm (
        "TIMERA0_ISR:\n" \
        ".global __isr_9\n" \
        "__isr_9:\n" \
        "mov.b #1, &isr_timer\n" \
        "bic #80, 0(r1)\n" \
        "reti");

//__attribute__((interrupt (TIMERA0_VECTOR), weak, naked))
//void TIMERA0_ISR(void)
//{
/*    isr_timer = 1;
    __bic_SR_register(LPM1_bits);*/
//}

int main(void)
{
    // Disable watchdog
    WDTCTL = WDTPW | WDTHOLD;

    // Set DCO frequency
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
    BCSCTL2 &= ~(BIT5|BIT4);  // Set MCLK prescaler to 1

    // Init debug UART
    uartInit();
    uartPutString("Nolleblink 2012\n\r");
    uartPutString("(c) ElektroTekniska Föreningen, 2009-2012\n\r\n\r");

    // Initialize SPI controller
    spiInit();

    // Init LED display
    ledInit();

    // Init application
    appInit();

    // Init radio
    radioInit();

    // System tick at 800 Hz
    TACCR0 = 2500;
    TACTL = TASSEL_2|MC_1|ID_3;
    TACCTL0 = CCIE;

    // Initialize Soft UART
    softu_init();

    int div = 0;

    while (1) {
        // Enable interrupts and go to sleep
        __bis_SR_register((LPM1_bits)|(GIE));

        // Something waked us up, handle it
        while (isr_timer | isr_radio | isr_uart) {
            // A system tick?
            if (isr_timer) {
                ledTick();

                if (++div == 800) {
                    P1OUT ^= 1;
                    appTick();
                    div = 0;
                }
                isr_timer = 0;
            }

            // A packet on the radio?
            if (isr_radio) {
                isr_radio = 0;
                radioISR();
                P1OUT ^= 2;
            }

            // Something from the bluetooth module?
            if (isr_uart) {
                isr_uart = 0;

                while (!FIFO_EMPTY(rx_fifo)) {
                    unsigned char c = FIFO_GET(rx_fifo);
                    bt_process(c);
                }
            }
        }
    }

    // we won't get here...
    return 0;
}
 
