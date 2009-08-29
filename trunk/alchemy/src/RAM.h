/*
 * RAM.h
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */

#ifndef RAM_H_
#define RAM_H_

#include "NES.h"

void initRAM(byte* rom, int size);
int getProgramROMLength();
void setBankStartAddress(int bankNum, int offsetInPRGROM);

#endif /* RAM_H_ */
