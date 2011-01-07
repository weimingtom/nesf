/*
 * RAM.h
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
#ifndef RAM_H_
#define RAM_H_
#include "NES.h"
#include <stdio.h>

int programROMLenght;

void initRAM(byte* rom, int size);
int getProgramROMLength();
void setBankStartAddress(int bankNum, int offsetInPRGROM);
void saveRAMState(FILE* file);
void loadRAMState(FILE* file);

#endif /* RAM_H_ */
