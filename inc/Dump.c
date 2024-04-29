// Dump.c
// Your solution to ECE319K Lab 3 Spring 2024
// Author: Esther Liu
// Last Modified: 2/12/24


#include <ti/devices/msp/msp.h>
#include "../inc/Timer.h"
#define MAXBUF 50
uint32_t DataBuffer[MAXBUF];
uint32_t TimeBuffer[MAXBUF];
uint32_t DebugCnt; // 0 to MAXBUF (0 is empty, MAXBUF is full)
uint32_t DataAND[MAXBUF];
// *****Debug_Init******
// Initializes your index or pointer.
// Input: none
// Output:none
void Debug_Init(void){
// students write this for Lab 3
// This function should also initialize Timer G12, call TimerG12_Init.
  DebugCnt = 0;
 // DataBuffer[DebugCnt];
 // TimeBuffer[DebugCnt];
  TimerG12_Init();
}

// *****Debug_Dump******
// Records one data and one time into the two arrays.
// Input: data is value to store in DataBuffer
// Output: 1 for success, 0 for failure (buffers full)
uint32_t Debug_Dump(uint32_t data){
    if(DebugCnt < MAXBUF) {
        DataBuffer[DebugCnt] = data;
        TimeBuffer[DebugCnt] = TIMG12->COUNTERREGS.CTR;
        DebugCnt++;
        return 1;
    }else{
return 0;
    }
}
// students write this for Lab 3
// The software simply reads TIMG12->COUNTERREGS.CTR to get the current time in bus cycles.



// *****Debug_Dump2******
// Always record data and time on the first call to Debug_Dump2
// However, after the first call
//    Records one data and one time into the two arrays, only if the data is different from the previous call.
//    Do not record data or time if the data is the same as the data from the previous call
// Input: data is value to store in DataBuffer
// Output: 1 for success (saved or skipped), 0 for failure (buffers full)
uint32_t Debug_Dump2(uint32_t data){
// optional for Lab 3
// The software simply reads TIMG12->COUNTERREGS.CTR to get the current time in bus cycles.

  return 1; // success
}
// *****Debug_Period******
// Calculate period of the recorded data using mask
// Input: mask specifies which bit(s) to observe
// Output: period in bus cycles
// Period is defined as rising edge (low to high) to the next rising edge.
// Return 0 if there is not enough collected data to calculate period .
uint32_t Debug_Period(uint32_t mask){
// students write this for Lab 3
// This function should not alter the recorded data.
// AND each recorded data with mask,
//    if nonzero the signal is considered high.
//    if zero, the signal is considered low.

    uint32_t i = 0;
    uint32_t check = 0;
    uint8_t rise = 1;
    uint8_t first = 1;
    uint32_t total = 0;
    uint8_t calculate = 0;
    uint8_t divider = 0;
    uint32_t period = 0;
    uint32_t value1 = 0;
    uint32_t value2 = 0;
    for(i=0; i < DebugCnt; i++){
        DataAND[i] = DataBuffer[i] & mask;
    }
    for(i=0; i < DebugCnt; i++){
            check = DataAND[i];
            if(check == 0){ //low
                rise = 1;
            }else if(check > 0 && rise == 1 && first == 1){ //high -- FIRST rising edge
                value1 = TimeBuffer[i];
                rise = 0;
                first = 0;
            }else if(check  > 0 && rise == 0){ //high -- nonrising edge
            }else if(check > 0 && rise == 1 && first == 0 && calculate == 0){ //high -- rising eduge NOT FIRST
                value2 = TimeBuffer[i];
                total = total + (value1 - value2);
                divider++;
                rise = 0;
                calculate = 1;
            }else if(check > 0 && rise == 1 && first == 0 && calculate == 1){
                value1 = value2;
                value2 = TimeBuffer[i];
                total = total + (value1 - value2);
                divider++;
                rise = 0;
        }
    }
    period = total / divider;
  return period; // average period
}


// *****Debug_Duty******
// Calculate duty cycle of the recorded data using mask
// Input: mask specifies which bit(s) to observe
// Output: period in percent (0 to 100)
// Period is defined as rising edge (low to high) to the next rising edge.
// High is defined as rising edge (low to high) to the next falling edge.
// Duty cycle is (100*High)/Period
// Return 0 if there is not enough collected data to calculate duty cycle.
uint32_t Debug_Duty(uint32_t mask){
// optional for Lab 3
// This function should not alter the recorded data.
// AND each recorded data with mask,
//    if nonzero the signal is considered high.
//    if zero, the signal is considered low.
    uint32_t i = 0;
    uint32_t check = 0;
    uint8_t rise = 1;
    uint8_t fall = 0;
    uint32_t value1 = 0;
    uint32_t value2 = 0;
    uint32_t total = 0;
    uint32_t divider = 0;
    for(i=0; i < DebugCnt; i++){
            DataAND[i] = DataBuffer[i] & mask;
    }
    //period
    uint32_t period_duty = Debug_Period(mask);

    //pulse width
    for(i=0; i < DebugCnt; i++){
        check = DataAND[i];
        if(check == 0 && fall == 1){ //falling edge
            value2 = TimeBuffer[i];
            total = total + (value1 - value2);
            divider++;
            fall = 0;
            rise = 1;
        }else if(check > 0 && rise == 1){ // rising edge
            value1 = TimeBuffer[i];
            rise = 0;
            fall = 1;
        }
    }
    if(divider == 0){
            return 0;
    }
    uint32_t pwm_duty = total/divider;
    uint32_t duty = (100 * pwm_duty)/period_duty;
    return duty;

  //return 42;
}

// Lab2 specific debugging code
uint32_t Theperiod;
void Dump(void){
  uint32_t out = GPIOB->DOUT31_0&0x0020000; // PB18-PB16 outputs
  uint32_t in = GPIOB->DIN31_0&0x0F;        // PB3-PB0 inputs
  uint32_t data = out|in;                   // PB18-PB16, PB3-PB0
  uint32_t result = Debug_Dump(data);       // calls your Lab3 function
  if(result == 0){ // 0 means full
    Theperiod = Debug_Period(1<<17); // calls your Lab3 function
    uint32_t duty_cycle = Debug_Duty(1<<17);
    //   __asm volatile("bkpt; \n"); // breakpoint here
// observe Theperiod
  }
}




