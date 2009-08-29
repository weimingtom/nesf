/*
 * Mapper.c
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
#include "Mapper.h"
#include "PPU.h"
#include "RAM.h"

extern int ppuVROMSize;
/**
 *
 * <P>
 * Method to access an address on the Mapper.
 * </P>
 *
 * @param address
 *            Address to access.
 * @param value
 *            Value to write to the address.
 *
 */
void accessMapper(int address, int value) {
}

/**
 *
 * <P>
 * Latch Used for MMC5
 * </P>
 *
 */
int PPU_Latch_RenderScreen(int mode, int addr) {
	return 0;
}

/**
 *
 * <P>
 * Method to access an address on the Mapper for writes from 0x4000 to
 * 0x5FFF.
 * </P>
 *
 * @param address
 *            Address to access.
 * @param value
 *            Value to write to the address.
 *
 */
void accessMapperLow(int address, int value) {
}

int accessMapperLowRead(int addr) {
	return (addr >> 8) & 0xFF;
}

/**
 *
 * <P>
 * Determine the number of the Memory Mapper.
 * </P>
 *
 * @return The number of the Memory Mapper.
 *
 */
int getMapperNumber() {
	return 0;
}

/**
 *
 * <P>
 * Latch the Memory Mapper.
 * </P>
 *
 * @param data
 *            A Mapper-specific data value that is passed to the Latch.
 *
 */
void mapperLatch(int data) {
}

/**
 *
 * <P>
 * Reset the Memory Mapper.
 * </P>
 *
 */
void resetMapper() {
	// Set the Program ROM
	int rombanks = getNum8KRomBanks();
	if (rombanks > 2) {
		setCPUBanks(0, 1, 2, 3);
	} else if (rombanks > 1) {
		setCPUBanks(0, 1, 0, 1);
	} else {
		setCPUBanks(0, 0, 0, 0);
	}
	// Set the Video ROM
	if (getNum1KVROMBanks() > 0) {
		setPPUBanks(0, 1, 2, 3, 4, 5, 6, 7);
	}
}
/**
 *
 * <P>
 * Syncronise the Memory Mapper Horizontally.
 * </P>
 *
 * @return Returns non-zero if a Mapper-specific interrupt occurred.
 *
 */
int syncH(int scanline) {
	return 0;
}

/**
 *
 * <P>
 * Syncronise the Memory Mapper Vertically.
 * </P>
 *
 * @return Returns non-zero if a Mapper-specific interrupt occurred.
 *
 */
int syncV() {
	return 0;
}

/********************************************************************************
 *
 * Standard Mapper Functions
 *
 *******************************************************************************/
/**
 *
 * Determine the Number of 8K ROM Banks in the Program ROM
 *
 */
int getNum8KRomBanks() {
	return (getProgramROMLength() >> 13);
}

/**
 *
 * Determine the Number of 1K ROM Banks in the Video ROM
 *
 */
int getNum1KVROMBanks() {
	return (ppuVROMSize >> 10);
}

/**
 *
 * Set the CPU Banks for 0x8000 - 0xFFFF
 *
 */
void setCPUBanks(int bank8, int bank9, int bankA, int bankB) {
	setBankStartAddress(0x8, (bank8 << 13)); // Bank 8
	setBankStartAddress(0x9, (bank8 << 13) + 0x1000); // Bank 9
	setBankStartAddress(0xA, (bank9 << 13)); // Bank A
	setBankStartAddress(0xB, (bank9 << 13) + 0x1000); // Bank B
	setBankStartAddress(0xC, (bankA << 13)); // Bank C
	setBankStartAddress(0xD, (bankA << 13) + 0x1000); // Bank D
	setBankStartAddress(0xE, (bankB << 13)); // Bank E
	setBankStartAddress(0xF, (bankB << 13) + 0x1000); // Bank F
}

/**
 *
 * Set the CPU Banks for 0x8000 - 0x9FFF
 *
 */
void setCPUBank8(int bank8) {
	setBankStartAddress(0x8, (bank8 << 13)); // Bank 8
	setBankStartAddress(0x9, (bank8 << 13) + 0x1000); // Bank 9
}

/**
 *
 * Set the CPU Banks for 0xA000 - 0xBFFF
 *
 */
void setCPUBankA(int bankA) {
	setBankStartAddress(0XA, (bankA << 13)); // Bank A
	setBankStartAddress(0XB, (bankA << 13) + 0x1000); // Bank B
}

/**
 *
 * Set the CPU Banks for 0xC000 - 0xDFFF
 *
 */
void setCPUBankC(int bankC) {
	setBankStartAddress(0xC, (bankC << 13)); // Bank C
	setBankStartAddress(0xD, (bankC << 13) + 0x1000); // Bank D
}

/**
 *
 * Set the CPU Banks for 0xC000 - 0xDFFF
 *
 */
void setCPUBankE(int bankE) {
	setBankStartAddress(0xE, (bankE << 13)); // Bank E
	setBankStartAddress(0xF, (bankE << 13) + 0x1000); // Bank F
}

/**
 *
 * Set the PPU Banks
 *
 */
void setPPUBanks(int bank0, int bank1, int bank2, int bank3, int bank4,
		int bank5, int bank6, int bank7) {
	setPPUBankStartAddress(0, bank0 << 10);
	setPPUBankStartAddress(1, bank1 << 10);
	setPPUBankStartAddress(2, bank2 << 10);
	setPPUBankStartAddress(3, bank3 << 10);
	setPPUBankStartAddress(4, bank4 << 10);
	setPPUBankStartAddress(5, bank5 << 10);
	setPPUBankStartAddress(6, bank6 << 10);
	setPPUBankStartAddress(7, bank7 << 10);
}

/**
 *
 * Set the PPU Bank 0
 *
 */
void setPPUBank0(int bankNum) {
	setPPUBankStartAddress(0, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 1
 *
 */
void setPPUBank1(int bankNum) {
	setPPUBankStartAddress(1, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 2
 *
 */
void setPPUBank2(int bankNum) {
	setPPUBankStartAddress(2, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 3
 *
 */
void setPPUBank3(int bankNum) {
	setPPUBankStartAddress(3, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 4
 *
 */
void setPPUBank4(int bankNum) {
	setPPUBankStartAddress(4, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 5
 *
 */
void setPPUBank5(int bankNum) {
	setPPUBankStartAddress(5, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 6
 *
 */
void setPPUBank6(int bankNum) {
	setPPUBankStartAddress(6, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 7
 *
 */
void setPPUBank7(int bankNum) {
	setPPUBankStartAddress(7, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 8
 *
 */
void setPPUBank8(int bankNum) {
	setPPUBankStartAddress(8, bankNum << 10);
}

/**
 *
 * Set the PPU Bank 9
 *
 */
void setPPUBank9(int bankNum) {
	setPPUBankStartAddress(9, bankNum << 10);
}

/**
 *
 * Set the PPU Bank A
 *
 */
void setPPUBankA(int bankNum) {
	setPPUBankStartAddress(10, bankNum << 10);
}

/**
 *
 * Set the PPU Bank B
 *
 */
void setPPUBankB(int bankNum) {
	setPPUBankStartAddress(11, bankNum << 10);
}

/**
 *
 * Set Mirroring Vertical
 *
 */
void setMirroringVertical() {
	setMirroring(0, 1, 0, 1);
}

/**
 *
 * Set Mirroring Horizontal
 *
 */
void setMirroringHorizontal() {
	setMirroring(0, 0, 1, 1);
}

/**
 *
 * Set Mirroring One-Screen High
 *
 */
void setMirroringOneScreenHigh() {
	setMirroring(1, 1, 1, 1);
}

/**
 *
 * Set Mirroring One-Screen Low
 *
 */
void setMirroringOneScreenLow() {
	setMirroring(0, 0, 0, 0);
}

/**
 *
 * Set Arbitary Mirroring
 *
 */
void setMirroring(int a, int b, int c, int d) {
	setPPUMirroring(a, b, c, d);
}

/**
 *
 * Set VRAM Bank
 *
 * Required for Mappers 1,4,5,6,13,19,80,85,96,119
 *
 */
void setVRAMBank(int bank, int banknum) {
	setPPUVRAMBank(bank, banknum);
}

/**
 *
 * Provide Mapper with CRC32 for Cartridge
 *
 */
void setCRC(long crc) {
}
