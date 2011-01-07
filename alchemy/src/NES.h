/*
 * NES.h
 *
 *  Created on: 2009-4-28
 *      Author: Administrator
 */

#ifndef NES_H_
#define NES_H_

#define true 1;
#define false 0;

typedef unsigned char byte;
typedef char boolean;
typedef unsigned short word;

#include "M6502.h"

int* connectScreen();
void cpuLoadRom(char* data);
void emulateFrame();


#endif /* NES_H_ */
