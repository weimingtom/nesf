/*
 * M6502.h
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */

#ifndef M6502_H_
#define M6502_H_

void emulateCPUCycles(float cycles);

void NMI();

void IRQ();

void resetCPU();

void correctCPUCycles();

void initResetCPU();

int readByte(int addr);

int readWord(int address);

void writeByte(int address, int value);

int byImmediate();

int byAbsolute();

int byAbsoluteY();

int byAbsoluteX();

int byZeroPage();

int byZeroPageX();

int byZeroPageY();

int byIndirectX();

int byIndirectY();

void checkPageBoundaryCrossing(int address1, int address2);

void setStatusFlags(int value);

int ASL(int i);

int LSR(int i);

int ROL(int i);

int ROR(int i);

int increment(int i);

int decrement(int i);

void operateAdd(int i);

void operateSub(int i);

void operateCmp(int i, int j);

void operateBit(int i);

void branch(int flagNum, boolean flagVal);

void push(int stackVal);

int pop();

void pushWord(int stackVal);

int popWord();

void instructionFetchExecute();

void eatCycles(int cycles);

#endif /* M6502_H_ */
