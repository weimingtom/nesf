
#include "Mapper.h"
#include "RAM.h"
#include "PPU.h"

int lastWriteAddr = 0;
int writeCount = 0;
int bits = 0;
int regs[4] = { 0, 0, 0, 0 };

int MMC1_bank1 = 0;
int MMC1_bank2 = 0;
int MMC1_bank3 = 0;
int MMC1_bank4 = 0;
int mmc1Size = 0;
int mmc1base256k = 0;
int mmc1Swap = 0;
int mmc1Hi1 = 0;
int mmc1Hi2 = 0;
int MMC1_1024K = 1024;
int MMC1_512K = 512;
int MMC1_SMALL = 1;

void MMC1_set_CPU_banks();
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
void accessMapper(int addr, int data)
{
	if (addr < 0x8000)
		return;
	// Reset Write Count if Register Changed
	if ((addr & 0x6000) != (lastWriteAddr & 0x6000)) {
		writeCount = 0;
		bits = 0x00;
	}
	// Record Last Register
	lastWriteAddr = addr;
	// Reset if Bit 7 Set
	if ((data & 0x80) != 0) {
		writeCount = 0;
		bits = 0x00;
		return;
	}
	// Set Bits
	if ((data & 0x01) != 0)
		bits |= (1 << writeCount);
	// Increment Write Count
	writeCount++;
	if (writeCount < 5)
		return;
	// Determine Register Number
	int reg_num = (addr & 0x7FFF) >> 13;
	// Write Bits to Register
	regs[reg_num] = bits;
	// Reset Write Count
	writeCount = 0;
	bits = 0x00;
	// Operate on Register
	switch (reg_num) {
	case 0: {
		// Set Mirroring
		if ((regs[0] & 0x02) != 0) {
			if ((regs[0] & 0x01) != 0) {
				setPPUMirroring(0,0,1,1);
			} else {
				setPPUMirroring(0,1,0,1);
			}
		} else {
			if ((regs[0] & 0x01) != 0) {
				setPPUMirroring(1, 1, 1, 1);
			} else {
				setPPUMirroring(0, 0, 0, 0);
			}
		}
	}
		break;
	case 1: {
		int bank_num = regs[1];
		if (mmc1Size == MMC1_1024K) {
			if ((regs[0] & 0x10) != 0) {
				if ((mmc1Swap) != 0) {
					mmc1base256k = (regs[1] & 0x10) >> 4;
					if ((regs[0] & 0x08) != 0) {
						mmc1base256k |= ((regs[2] & 0x10) >> 3);
					}
					MMC1_set_CPU_banks();
					mmc1Swap = 0;
				} else {
					mmc1Swap = 1;
				}
			} else {
				mmc1base256k = ((regs[1] & 0x10) != 0) ? 3 : 0;
				MMC1_set_CPU_banks();
			}
		} else if ((mmc1Size == MMC1_512K) && (getNum1KVROMBanks() == 0)) {
			mmc1base256k = (regs[1] & 0x10) >> 4;
			MMC1_set_CPU_banks();
		} else if (getNum1KVROMBanks() != 0) {
			// Set VROM Banks
			if ((regs[0] & 0x10) != 0) {
				// Swap 4K
				bank_num <<= 2;
				setPPUBank0(bank_num + 0);
				setPPUBank1(bank_num + 1);
				setPPUBank2(bank_num + 2);
				setPPUBank3(bank_num + 3);
			} else {
				// Swap 8K
				bank_num <<= 2;
				setPPUBanks(bank_num + 0, bank_num + 1, bank_num + 2, bank_num
						+ 3, bank_num + 4, bank_num + 5, bank_num + 6, bank_num
						+ 7);
			}
		} else {
			if ((regs[0] & 0x10) != 0) {
				bank_num <<= 2;
				setVRAMBank(0, bank_num + 0);
				setVRAMBank(1, bank_num + 1);
				setVRAMBank(2, bank_num + 2);
				setVRAMBank(3, bank_num + 3);
			}
		}
	}
		break;
	case 2: {
		int bank_num = regs[2];
		if ((mmc1Size == MMC1_1024K) && ((regs[0] & 0x08) != 0)) {
			if ((mmc1Swap) != 0) {
				mmc1base256k = (regs[1] & 0x10) >> 4;
				mmc1base256k |= ((regs[2] & 0x10) >> 3);
				MMC1_set_CPU_banks();
				mmc1Swap = 0;
			} else {
				mmc1Swap = 1;
			}
		}
		if (getNum1KVROMBanks() == 0) {
			if ((regs[0] & 0x10) != 0) {
				bank_num <<= 2;
				setVRAMBank(4, bank_num + 0);
				setVRAMBank(5, bank_num + 1);
				setVRAMBank(6, bank_num + 2);
				setVRAMBank(7, bank_num + 3);
				break;
			}
		}
		if ((regs[0] & 0x10) != 0) {
			// Swap 4K
			bank_num <<= 2;
			setPPUBank4(bank_num + 0);
			setPPUBank5(bank_num + 1);
			setPPUBank6(bank_num + 2);
			setPPUBank7(bank_num + 3);
		}
	}
		break;
	case 3: {
		// Set Program ROM Bank
		int bank_num = regs[3];
		if ((regs[0] & 0x08) != 0) {
			// 16K of ROM
			bank_num <<= 1;
			if ((regs[0] & 0x04) != 0) {
				// 16K of ROM at $8000
				MMC1_bank1 = bank_num;
				MMC1_bank2 = bank_num + 1;
				MMC1_bank3 = mmc1Hi1;
				MMC1_bank4 = mmc1Hi2;
			} else {
				// 16K of ROM at $C000
				if (mmc1Size == MMC1_SMALL) {
					MMC1_bank1 = 0;
					MMC1_bank2 = 1;
					MMC1_bank3 = bank_num;
					MMC1_bank4 = bank_num + 1;
				}
			}
		} else {
			// 32K of ROM at $8000
			bank_num <<= 1;
			MMC1_bank1 = bank_num;
			MMC1_bank2 = bank_num + 1;
			if (mmc1Size == MMC1_SMALL) {
				MMC1_bank3 = bank_num + 2;
				MMC1_bank4 = bank_num + 3;
			}
		}
		MMC1_set_CPU_banks();
	}
		break;
	}
}

void accessMapperLow(int address, int value) {
}

int accessMapperLowRead(int addr) {
    return (addr >> 8) & 0xFF;
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
	// Reset the Write Counters
	writeCount = 0;
	bits = 0x00;
	// Reset the Registers
	regs[0] = 0x0C;
	regs[1] = 0x00;
	regs[2] = 0x00;
	regs[3] = 0x00;
	// Determine the Size in K of the Program ROM
	int size_in_K = getNum8KRomBanks() * 8;
	if (size_in_K == 1024) {
		mmc1Size = MMC1_1024K;
	} else if (size_in_K == 512) {
		mmc1Size = MMC1_512K;
	} else {
		mmc1Size = MMC1_SMALL;
	}
	// Select the First 256K
	mmc1base256k = 0;
	mmc1Swap = 0;
	// Map the High Pages
	if (mmc1Size == MMC1_SMALL) {
		// Set two High Pages to Last two Banks
		mmc1Hi1 = getNum8KRomBanks() - 2;
		mmc1Hi2 = getNum8KRomBanks() - 1;
	} else {
		// Set two High Pages to Last Two Banks of Current 256K Region
		mmc1Hi1 = (256 / 8) - 2;
		mmc1Hi2 = (256 / 8) - 1;
	}
	// Set CPU Banks
	MMC1_bank1 = 0;
	MMC1_bank2 = 1;
	MMC1_bank3 = mmc1Hi1;
	MMC1_bank4 = mmc1Hi2;
	MMC1_set_CPU_banks();
	// Set PPUMemory Bank Addresses
	setPPUBanks(0, 1, 2, 3, 4, 5, 6, 7);
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
    return programROMLenght >> 13;
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

void MMC1_set_CPU_banks() {
	setCPUBanks((mmc1base256k << 5) + (MMC1_bank1 & ((256 / 8) - 1)),
			(mmc1base256k << 5) + (MMC1_bank2 & ((256 / 8) - 1)), (mmc1base256k
					<< 5) + (MMC1_bank3 & ((256 / 8) - 1)), (mmc1base256k << 5)
					+ (MMC1_bank4 & ((256 / 8) - 1)));
}

void saveMapperState(FILE* file)
{
//    fwrite(&regs,sizeof(int),8, file);
//
//    fputc( irq_counter & 0xFF, file);
//    fputc( irq_enabled ? 0xFF : 0x00, file);
//    fputc( irq_latch & 0xFF, file);
//    fputc( prg0  & 0xFF, file);
//    fputc( prg1  & 0xFF, file);
//    fputc( chr01 & 0xFF, file);
//    fputc( chr23 & 0xFF, file);
//    fputc( chr4  & 0xFF, file);
//    fputc( chr5  & 0xFF, file);
//    fputc( chr6  & 0xFF, file);
//    fputc( chr7  & 0xFF, file);
}
void loadMapperState(FILE* file)
{
//    fread(&regs,sizeof(int),8, file);
//
//    irq_counter = fgetc(file) & 0xFF;
//    irq_enabled = (fgetc(file) == 0xFF)? 1: 0;
//    irq_latch = fgetc(file) & 0xFF;
//    prg0 = fgetc(file) & 0xFF;
//    prg1 = fgetc(file) & 0xFF;
//    chr01= fgetc(file) & 0xFF;
//    chr23= fgetc(file) & 0xFF;
//    chr4 = fgetc(file) & 0xFF;
//    chr5 = fgetc(file) & 0xFF;
//    chr6 = fgetc(file) & 0xFF;
//    chr7 = fgetc(file) & 0xFF;
}
