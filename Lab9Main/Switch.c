/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    IOMUX->SECCFG.PINCM[PA24INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA26INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA25INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA31INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA9INDEX] = 0x00040081;
}
// return current state of switches
int data;
int Switch_In(int button){
    if(button == (1<<13)) {
        data = GPIOB->DIN31_0;
    }else {
        data = GPIOA->DIN31_0;
    }
    data &= button;
    return data;
}
