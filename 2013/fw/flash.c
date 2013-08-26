/*
 *  flash.c
 *  Nolleblink 2010 | Blinkmojt
 *
 *  Modified by Fredrik Ahlberg on 2010-08-22.
 *  Original implementation by Joakim Arnsby.
 *  Copyright (c) 2010 ETF. All rights reserved.
 *
 */

#include <msp430.h>
#include <string.h>

#include "flash.h"

char flash_read(char *buf, unsigned char len)
{
    if (len > FLASH_MAIN_SIZE) {
        len = FLASH_MAIN_SIZE;
    }
    
    unsigned short checksum = 0, idx;

    for (idx=0; idx<FLASH_MAIN_SIZE; idx++) {
        checksum += FLASH_MAIN_BASE[idx];
    }

    if (((checksum & 0xff) != FLASH_MAIN_BASE[FLASH_MAIN_SIZE])
            || ((checksum >> 8) != FLASH_MAIN_BASE[FLASH_MAIN_SIZE+1])) {
        return 0;
    }

    memcpy(buf, FLASH_MAIN_BASE, len);

    return 1;
}

void flash_write(const char *buf, unsigned char len)
{
    //BCSCTL1 = CALBC1_1MHZ;    // Set DCO to 1MHz
    //DCOCTL = CALDCO_1MHZ;
    
    FCTL2 = FWKEY|FSSEL0|63;   // MCLK/3 for Flash Timing Generator
    FCTL3 = FWKEY;              // Clear Lock bit
    FCTL1 = FWKEY|ERASE;        // Set Erase bit
    FLASH_MAIN_BASE[0] = 0;     // Dummy write to erase Flash seg
    FCTL3 = FWKEY;              // Clear Lock bit
    FCTL1 = FWKEY|ERASE;        // Set Erase bit
    FLASH_MAIN_BASE[64] = 0;    // Dummy write to erase Flash seg
    FCTL1 = FWKEY|WRT;          // Set WRT bit for write operation
	
    memcpy(FLASH_MAIN_BASE, buf, len);

    unsigned short checksum = 0, idx;

    for (idx=0; idx<FLASH_MAIN_SIZE; idx++) {
        checksum += FLASH_MAIN_BASE[idx];
    }

    FLASH_MAIN_BASE[FLASH_MAIN_SIZE] = checksum & 0xff;
    FLASH_MAIN_BASE[FLASH_MAIN_SIZE+1] = checksum >> 8;

    FCTL1 = FWKEY;              // Clear WRT bit
    FCTL3 = FWKEY|LOCK;         // Set LOCK bit

    //BCSCTL1 = CALBC1_16MHZ;   // Set DCO to 16MHz
    //DCOCTL = CALDCO_16MHZ;
    //BCSCTL2 &= ~(BIT5|BIT4);
}

