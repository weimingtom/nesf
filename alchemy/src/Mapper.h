/*
 * Mapper.h
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */

#ifndef MAPPER_H_
#define MAPPER_H_

void accessMapper(int address, int value);

void accessMapperLow(int address, int value);

int accessMapperLowRead(int addr);

int getMapperNumber();

void mapperLatch(int data);

void resetMapper();

int syncH(int scanline);

int syncV();

int getNum8KRomBanks();

int getNum1KVROMBanks();

void setCPUBanks(int bank8, int bank9, int bankA, int bankB);

void setCPUBank8(int bank8);

void setCPUBankA(int bankA);

void setCPUBankC(int bankC);

void setCPUBankE(int bankE);

void setPPUBanks(int bank0, int bank1, int bank2, int bank3, int bank4,
		int bank5, int bank6, int bank7);

void setPPUBank0(int bankNum);

void setPPUBank1(int bankNum);

void setPPUBank2(int bankNum);

void setPPUBank3(int bankNum);

void setPPUBank4(int bankNum);

void setPPUBank5(int bankNum);

void setPPUBank6(int bankNum);

void setPPUBank7(int bankNum);

void setPPUBank8(int bankNum);

void setPPUBank9(int bankNum);

void setPPUBankA(int bankNum);

void setPPUBankB(int bankNum);

void setMirroringVertical();

void setMirroringHorizontal();

void setMirroringOneScreenHigh();

void setMirroringOneScreenLow();

void setMirroring(int a, int b, int c, int d);

void setVRAMBank(int bank, int banknum);

void setCRC(long crc);

#endif /* MAPPER_H_ */
