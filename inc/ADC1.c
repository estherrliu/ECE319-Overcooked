/* ADC1.c
 * Students put your names here
 * Modified: put the date here
 * 12-bit ADC input on ADC1 channel 5, PB18
 */
#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"
#include "../inc/ADC.h"
#define ADCVREF_VDDA 0x000
#define ADCVREF_INT  0x200

//need to add ADC.h for reference

//joystick1 -- ADC1, PA18 vert(channel3), PB17 horiz(channel4)
//joystick0 -- ADC0, PB24 vert(channel5), PA27 horiz(channel0)

// Assumes 40 MHz bus
void ADC_InitDual(ADC12_Regs *adc12,uint32_t channel1,uint32_t channel2, uint32_t reference){
    // Reset ADC and VREF
    // RSTCLR
    //   bits 31-24 unlock key 0xB1
    //   bit 1 is Clear reset sticky bit
    //   bit 0 is reset ADC
  adc12->ULLMEM.GPRCM.RSTCTL = (uint32_t)0xB1000003;
  if(reference == ADCVREF_INT){
    VREF->GPRCM.RSTCTL = (uint32_t)0xB1000003;
  }
    // Enable power ADC and VREF
    // PWREN
    //   bits 31-24 unlock key 0x26
    //   bit 0 is Enable Power
  adc12->ULLMEM.GPRCM.PWREN = (uint32_t)0x26000001;
  if(reference == ADCVREF_INT){
    VREF->GPRCM.PWREN = (uint32_t)0x26000001;
  }
  Clock_Delay(24); // time for ADC and VREF to power up
  adc12->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // ULPCLK
  // bits 31-24 key=0xA9
  // bit 5 CCONSTOP= 0 not continuous clock in stop mode
  // bit 4 CCORUN= 0 not continuous clock in run mode
  // bit 1-0 0=ULPCLK,1=SYSOSC,2=HFCLK
  adc12->ULLMEM.CLKFREQ = 7; // 40 to 48 MHz
  adc12->ULLMEM.CTL0 = 0x03010000;
  // bits 26-24 = 011 divide by 8
  // bit 16 PWRDN=1 for manual, =0 power down on completion, if no pending trigger
  // bit 0 ENC=0 disable (1 to 0 will end conversion)
  adc12->ULLMEM.CTL1 = 0x00010000;
  // bits 30-28 =0  no shift
  // bits 26-24 =0  no averaging
  // bit 20 SAMPMODE=0 high phase
  // bits 17-16 CONSEQ=01 ADC at start to end
  // bit 8 SC=0 for stop, =1 to software start
  // bit 0 TRIGSRC=0 software trigger
  adc12->ULLMEM.CTL2 = 0x02010000;
  // bits 28-24 ENDADD=2 (which  MEMCTL to end)
  // bits 20-16 STARTADD=1 (which  MEMCTL to start)
  // bits 15-11 SAMPCNT (for DMA)
  // bit 10 FIFOEN=0 disable FIFO
  // bit 8  DMAEN=0 disable DMA
  // bits 2-1 RES=0 for 12 bit (=1 for 10bit,=2for 8-bit)
  // bit 0 DF=0 unsigned formant (1 for signed, left aligned)
  adc12->ULLMEM.MEMCTL[1] = reference+channel1;
  adc12->ULLMEM.MEMCTL[2] = reference+channel2;
  // bit 28 WINCOMP=0 disable window comparator
  // bit 24 TRIG trigger policy, =0 for auto next, =1 for next requires trigger
  // bit 20 BCSEN=0 disable burn out current
  // bit 16 = AVGEN =0 for no averaging
  // bit 12 = STIME=0 for SCOMP0
  // bits 9-8 VRSEL = 10 for internal VREF,(00 for VDDA)
  // bits 4-0 channel = 0 to 7 available
  adc12->ULLMEM.SCOMP0 = 0; // 8 sample clocks
//  adc12->ULLMEM.GEN_EVENT.ICLR |= 0x0100; // clear flag MEMCTL[1] ??
  adc12->ULLMEM.GEN_EVENT.IMASK = 0; // no interrupt
  if(reference == ADCVREF_INT){
    VREF->CLKSEL = 0x00000008; // bus clock
    VREF->CLKDIV = 0; // divide by 1
    VREF->CTL0 = 0x0001;
  // bit 8 SHMODE = off
  // bit 7 BUFCONFIG=0 for 2.4 (=1 for 1.4)
  // bit 0 is enable
    VREF->CTL2 = 0;
  // bits 31-16 HCYCLE=0
    // bits 15-0 SHCYCLE=0
    while((VREF->CTL1&0x01)==0){}; // wait for VREF to be ready
  }
}

// sample 12-bit ADC
void ADC_InDual(ADC12_Regs *adc12, uint32_t *d1, uint32_t *d2){
  adc12->ULLMEM.CTL0 |= 0x00000001; // enable conversions
  adc12->ULLMEM.CTL1 |= 0x00000100; // start ADC
  uint32_t volatile delay=adc12->ULLMEM.STATUS; // time to let ADC start
  while((adc12->ULLMEM.STATUS&0x01)==0x01){}; // wait for completion
  *d1 = adc12->ULLMEM.MEMRES[2];
  *d2 = adc12->ULLMEM.MEMRES[1];
}


void ADC0init(void){
// write code to initialize ADC1 channel 5, PB18
// Your measurement will be connected to PB18
// 12-bit mode, 0 to 3.3V, right justified
// software trigger, no averaging
    ADC0->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) reset
    ADC0->ULLMEM.GPRCM.PWREN = 0x26000001;  // 2) activate
    Clock_Delay(24);                        // 3) wait
    ADC0->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // 4) ULPCLK
    ADC0->ULLMEM.CLKFREQ = 7;               // 5) 40-48 MHz
    ADC0->ULLMEM.CTL0 = 0x03010000;         // 6) divide by 8
    ADC0->ULLMEM.CTL1 = 0x00000000;         // 7) mode
    ADC0->ULLMEM.CTL2 = 0x00000000;         // 8) MEMRES
    //ADC0->ULLMEM.MEMCTL[0] = 5;             // 9) channel 5 is PB24
    ADC0->ULLMEM.SCOMP0 = 0;                // 10) 8 sample clocks
    ADC0->ULLMEM.CPU_INT.IMASK = 0;         // 11) no interrupt
}

void ADC1init(void){
// write code to initialize ADC1 channel 5, PB18
// Your measurement will be connected to PB18
// 12-bit mode, 0 to 3.3V, right justified
// software trigger, no averaging
    ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) reset
    ADC1->ULLMEM.GPRCM.PWREN = 0x26000001;  // 2) activate
    Clock_Delay(24);                        // 3) wait
    ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // 4) ULPCLK
    ADC1->ULLMEM.CLKFREQ = 7;               // 5) 40-48 MHz
    ADC1->ULLMEM.CTL0 = 0x03010000;         // 6) divide by 8
    ADC1->ULLMEM.CTL1 = 0x00000000;         // 7) mode
    ADC1->ULLMEM.CTL2 = 0x00000000;         // 8) MEMRES
    //ADC1->ULLMEM.MEMCTL[0] = 3;             // 9) channel 3 is PA18 //PROBLEM HAS SMTH TO DO WITH INITIALIZING VERT IN INIT
    ADC1->ULLMEM.SCOMP0 = 0;                // 10) 8 sample clocks
    ADC1->ULLMEM.CPU_INT.IMASK = 0;         // 11) no interrupt
}

uint32_t JS0in(int channel){
  // write code to sample ADC0
  // return digital result (0 to 4095)
    ADC0->ULLMEM.CTL0 |= 0x00000001;             // 1) enable conversions
    ADC0->ULLMEM.CTL1 |= 0x00000100;             // 2) start ADC
    uint32_t volatile delay=ADC0->ULLMEM.STATUS; // 3) time to let ADC start
    ADC0->ULLMEM.MEMCTL[0] = channel;
    while((ADC0->ULLMEM.STATUS&0x01)==0x01){}    // 4) wait for completion
    return ADC0->ULLMEM.MEMRES[0];               // 5) 12-bit result
}

uint32_t JS1in(int channel){
  // write code to sample ADC1
  // return digital result (0 to 4095)
    ADC1->ULLMEM.CTL0 |= 0x00000001;             // 1) enable conversions
    ADC1->ULLMEM.CTL1 |= 0x00000100;             // 2) start ADC
    uint32_t volatile delay=ADC1->ULLMEM.STATUS; // 3) time to let ADC start
    ADC1->ULLMEM.MEMCTL[0] = channel;
    while((ADC1->ULLMEM.STATUS&0x01)==0x01){}    // 4) wait for completion
    return ADC1->ULLMEM.MEMRES[0];               // 5) 12-bit result
}

// your function to convert ADC sample to distance (0.001cm)
// use main2 to calibrate the system fill in 5 points in Calibration.xls
//    determine constants k1 k2 to fit Position=(k1*Data + k2)>>12
uint32_t Convert(uint32_t input){
    uint32_t position = (2001*input + 110)>>12;
  return position; // replace this with a linear function
}

// do not use this function
// it is added just to show you how SLOW floating point in on a Cortex M0+
float FloatConvert(uint32_t input){
  return 0.00048828125*input -0.0001812345;
}

