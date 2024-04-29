// FIFO1.c
// Runs on any microcontroller
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 10/29/2023
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>


// Declare state variables for FiFo
//        size, buffer, put and get indexes


// *********** Fifo1_Init**********
// Initializes a software FIFO1 of a
// fixed size and sets up indexes for
// put and get operations
#define FIFO_SIZE 7
static int32_t PutI;  // index to put new
static int32_t GetI;  // index of oldest
static int32_t Fifo[FIFO_SIZE];
void Fifo1_Init(void){
    /*for(int i = 0; i<7; i++){
        Fifo[i] = 0;
    }*/
    PutI = GetI = 0;
//Complete this
}

// *********** Fifo1_Put**********
// Adds an element to the FIFO1
// Input: data is character to be inserted
// Output: 1 for success, data properly saved
//         0 for failure, FIFO1 is full

int Fifo1_Put(char data){
  //Complete this routine
    if(((PutI+1)%FIFO_SIZE) == GetI) return 0; // fail if full
    Fifo[PutI] = data;         // save in Fifo
    PutI = (PutI+1)%FIFO_SIZE; // next place to put
    return 1;

}

// *********** Fifo1_Get**********
// Gets an element from the FIFO1
// Input: none
// Output: If the FIFO1 is empty return 0
//         If the FIFO1 has data, remove it, and return it
int Fifo1_Get(void){
    if(GetI == PutI) return 0; // fail if empty
    char data = Fifo[GetI];      // retrieve data
    GetI = (GetI+1)%FIFO_SIZE; // next place to get
    return data;
}



