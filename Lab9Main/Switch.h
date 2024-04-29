/*
 * Switch.h
 *
 *  Created on: Nov 5, 2023
 *      Author: jonat
 */

#ifndef SWITCH_H_
#define SWITCH_H_
#include <stdint.h>

// initialize your switches
void Switch_Init(void);

// return current state of switches
int Switch_In(int button);


#endif /* SWITCH_H_ */
