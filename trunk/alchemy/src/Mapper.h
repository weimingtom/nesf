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
void setVRAMBank(int bank, int banknum);
void setCRC(long crc);


void MMC3_set_CPU_banks();
void MMC3_set_PPU_banks();
int chr_swap();
int prg_swap();
/**
 * Set the PPU Bank 0
 */
#define setPPUBank0(bankNum)\
    setPPUBankStartAddress(0, bankNum << 10)
/**
 * Set the PPU Bank 1
 */
#define setPPUBank1(bankNum)\
    setPPUBankStartAddress(1, bankNum << 10)
/**
 * Set the PPU Bank 2
 */
#define setPPUBank2(bankNum)\
    setPPUBankStartAddress(2, bankNum << 10)
/**
 * Set the PPU Bank 3
 */
#define setPPUBank3(bankNum)\
    setPPUBankStartAddress(3, bankNum << 10)
/**
 * Set the PPU Bank 4
 */
#define setPPUBank4(bankNum)\
    setPPUBankStartAddress(4, bankNum << 10)
/**
 * Set the PPU Bank 5
 */
#define setPPUBank5(bankNum)\
    setPPUBankStartAddress(5, bankNum << 10)
/**
 * Set the PPU Bank 6
 */
#define setPPUBank6(bankNum)\
    setPPUBankStartAddress(6, bankNum << 10)
/**
 * Set the PPU Bank 7
 */
#define setPPUBank7(bankNum)\
    setPPUBankStartAddress(7, bankNum << 10)
/**
 * Set the PPU Bank 8
 */
#define setPPUBank8(bankNum)\
    setPPUBankStartAddress(8, bankNum << 10)
/**
 * Set the PPU Bank 9
 */
#define setPPUBank9(bankNum)\
    setPPUBankStartAddress(9, bankNum << 10)
/**
 * Set the PPU Bank A
 */
#define setPPUBankA(bankNum)\
    setPPUBankStartAddress(10, bankNum << 10)
/**
 * Set the PPU Bank B
 */
#define setPPUBankB(bankNum)\
    setPPUBankStartAddress(11, bankNum << 10)

#endif /* MAPPER_H_ */
