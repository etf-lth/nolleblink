#include <msp430.h>
#include "board.h"
#include "fifo.h"

/* 1200 baud at 16MHz */
//#define BITTIME 1667
//#define BITTIME15 2500

/* 9600 baud at 16MHz */
#define BITTIME 208
#define BITTIME15 312

FIFO_DEF(rx_fifo);

static unsigned char softu_dir;
static unsigned char softu_state;
static unsigned char softu_buf;

extern volatile unsigned char isr_uart;

__attribute__((interrupt (TIMERB0_VECTOR)))
void TIMERB_ISR(void)
{
    if (softu_dir) {
        if (!softu_state) {
            softu_dir = 0;
            TBCCTL0 = 0;
            BT_RX_PxIFG &= ~BT_RX_PIN;
            BT_RX_PxIE |= BT_RX_PIN;
        } else {
            BT_TX_PxOUT &= ~BT_TX_PIN;
            if (softu_buf & softu_state)
                BT_TX_PxOUT |= BT_TX_PIN;
            softu_state <<= 1;
            TBR = 0;
            TBCCR0 = BITTIME;
        }
    } else {
        if (!softu_state) {
            TBCCTL0 = 0;

            FIFO_PUT(rx_fifo, softu_buf);
            isr_uart = 1;
            __bic_SR_register_on_exit(LPM1_bits);

            BT_RX_PxIFG &= ~BT_RX_PIN;
            BT_RX_PxIE |= BT_RX_PIN;
        } else {
            if (BT_RX_PxIN & BT_RX_PIN) {
                softu_buf |= softu_state;
            }
            softu_state <<= 1;
            TBR = 0;
            TBCCR0 = BITTIME;
        }
    }
}

void softu_interrupt(void)
{
    if (BT_RX_PxIFG & BT_RX_PIN) {
        softu_buf = 0;
        softu_state = 1;
        BT_RX_PxIE &= ~BT_RX_PIN;
        TBR = 0;
        TBCCR0 = BITTIME15;
        TBCCTL0 = CCIE;

        BT_RX_PxIFG &= ~BT_RX_PIN;
    }
}

void softu_transmit(unsigned char data)
{
    softu_buf = data;
    softu_state = 1;
    softu_dir = 1;

    BT_TX_PxOUT &= ~BT_TX_PIN;
    TBR = 0;
    TBCCR0 = BITTIME;
    TBCCTL0 = CCIE;
    BT_RX_PxIE &= ~BT_RX_PIN;
}

void softu_init(void)
{
    // Soft UART for Bluetooth
    BT_RX_PxSEL &= ~BT_RX_PIN;
    BT_RX_PxDIR &= ~BT_RX_PIN;
    BT_RX_PxIES |= BT_RX_PIN;
    BT_RX_PxIFG &= ~BT_RX_PIN;
    BT_RX_PxIE |= BT_RX_PIN;
    BT_TX_PxDIR |= BT_TX_PIN;
    BT_TX_PxOUT |= BT_TX_PIN;
    TBCTL = TBSSEL_2|MC_1|ID_3; // SMCLK / 8
}

