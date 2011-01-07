/*
 * PPU.h
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
#ifndef PPU_H_
#define PPU_H_
#include "NES.h"
#include <stdio.h>

int REG_2000;
int REG_2001;
int REG_2002;
int mirrorFourScreen;
int mirrorHorizontal;
int mirrorVertical;
int spriteMemory[256];
int frameIRQEnabled;
int ppuVROMSize;

void setPPUlatchMapper(boolean v);
void backgroundBlitzer();
void spriteBlitzer(int lineNum);
void startVBlank();
void endVBlank();
void endFrame();
boolean nmiEnabled();
void drawScanLine();
void startFrame();
void loopyScanlineEnd();
void loopyScanlineStart();
int readFromPPU(int address);
boolean renderLine();
void writeToPPU(int address, int value);
void initPPU(byte* rom, int size, boolean vMirroring, boolean fourScreenNT);
int ppuRead(int addr);
int readPatternTable(int addr);
void reset2005Toggle();
void set2005(int value);
void set2006(int value);
void setMirror();
void setPPUMirroring(int nt0, int nt1, int nt2, int nt3);
void ppuWrite(int value);
int ppuVRAM(int addr);
void setPPUBankStartAddress(int bankNum, int offsetInVROM);
void setPPUVRAMBank(int bank, int banknum);

void savePPUState(FILE* file);
void loadPPUState(FILE* file);

#endif /* PPU_H_ */
