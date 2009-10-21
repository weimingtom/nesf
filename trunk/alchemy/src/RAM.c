#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RAM.h"
#include "NES.h"
#include "Mapper.h"
#include "M6502.h"
#include "PPU.h"
#include "JoyPad.h"

byte RAM[0x0800];
byte* programROM;

#define BANK_LENGTH 16
int programROMLenght;

int bank[BANK_LENGTH]; // Each bank is 0x1000

extern int spriteMemory[256];
extern int frameIRQEnabled;

/**
 *
 * <P>
 * Initialise the Memory Manager.
 * </P>
 *
 */
void initRAM(byte* progROM, int size) {
	// Fetch the Program ROM
	programROM = progROM;
	programROMLenght = size;

	// Initialise the PPU Memory
	// Clear the Main Memory
	memset (RAM, 0, sizeof(int)*0x0800);
	// Load the Trainer ROM if it Exists
	frameIRQEnabled = 0xFF;
}

/**
 *
 * <P>
 * Sets the offset in Program ROM of a Memory Bank.
 * </P>
 *
 * @param bankNum
 *            The bank number to configure (0-15).
 * @param offsetInPRGROM
 *            The offset in Program ROM that the bank starts at.
 *
 */
void setBankStartAddress(int bankNum, int offsetInPRGROM) {
	offsetInPRGROM %= programROMLenght;
	bank[bankNum % BANK_LENGTH] = offsetInPRGROM;
}

/**
 *
 * <P>
 * Read from Memory.
 * </P>
 *
 * @return The value at the specified address.
 *
 */
int readByte(int addr) {
	int x;
	switch (addr >> 13) {
	case 0:
		// RAM Mirrored 4 Times
		return RAM[addr & 0x7FF];
	case 1:
		return readFromPPU(addr & 0xE007);
	case 2:
		if (addr < 0x4000) {
			// Input/Output
		} else if (addr < 0x4016) {
			// SPR-RAM DMA
			if (addr == 0x4014) {
				return 0;
			} else if (addr == 0x4015 && ((frameIRQEnabled & 0xC0) == 0)) {
				return 40;
			}
			return 0;
		} else if (addr < 0x4018) {
			// High I/O Regs
			if (addr == 0x4016) {
				// Joypad #1
				return readJoyPadBit();
			} else if (addr == 0x4017) {
				// Joypad #2
				return readJoyPadBit();
			}
			return 0;
		} else if (addr < 0x6000) {
			// Expansion ROM
			return accessMapperLowRead(addr);
		}
	case 3:
		// 0x6000 - 0x7FFF SaveRAM
		return 0;

	default:
		// Get Offset
		x = bank[((addr & 0xF000) >> 12)];
		// Return Memory
		return programROM[x + (addr & 0x0FFF)];
	}
}

/**
 *
 * <P>
 * Write to Memory.
 * </P>
 *
 */
void writeByte(int addr, int value) {
	int i;
	byte* source;
	switch (addr >> 13) {
	case 0:
		// 0x0000 - 0x1FFF RAM
		RAM[addr & 0x7FF] = value;
		return;
	case 1:
		// Low IO Registers
		writeToPPU(addr & 0xE007, value);
		return;
	case 2:
		if (addr < 0x4018) {
			// High IO Registers
			accessMapperLow(addr, value);
			switch (addr) {
			case 0x4000:
			case 0x4001:
			case 0x4002:
			case 0x4003:
			case 0x4004:
			case 0x4005:
			case 0x4006:
			case 0x4007:
			case 0x4008:
			case 0x4009:
			case 0x400A:
			case 0x400B:
			case 0x400C:
			case 0x400D:
			case 0x400E:
			case 0x400F:
			case 0x4010:
			case 0x4011:
			case 0x4012:
			case 0x4013:
				return;
			case 0x4014: // Sprite DMA Register
				source = RAM;
				int k = value << 8;
				switch (k & 0xF000) {
				case 0x8000: // DMA Transfer from Program ROM
					source = programROM;
					k = bank[(k >> 12) + (k & 0x0FFF)];
					break;
				case 0x6000: // DMA Transfer from SaveRAM
				case 0x7000:
					return;
				case 0x5000: // DMA Transfer from Expansion RAM
					return;
				case 0x2000: // DMA Transfer from Registers
				case 0x3000:
				case 0x4000:
					return;
				case 0x0000: // DMA Transfer from RAM
				case 0x1000:
					source = RAM;
					k &= 0x7FF;
					break;
				}
				// Perform the DMA Transfer
				for (i = 0; i < 256; i++) {
					spriteMemory[i] = source[k] & 0xFF;
					k++;
				}
				// Burn Some Cycles
				eatCycles(514);
				return;
			case 0x4015:
				return;
			case 0x4016: // Joypad #1
				if ((value & 0x1) == 0)
					resetJoyPad();
				return;
			case 0x4017: // Joypad #2
				if ((value & 0x1) == 0)
					resetJoyPad();
				return;
			}
			return;
		} else if (addr < 0x6000) {
			// Expansion ROM and Low Mapper Write Region
			accessMapperLow(addr, value);
		}
	case 3:
		// Save RAM
		accessMapper(addr, value);
		return;
	default:
		accessMapper(addr, value);
	}
}

/**
 *
 * <P>
 * Read a 16 bit Word from Memory.
 * </P>
 *
 */
int readWord(int address) {
	int low = readByte(address);
	int high = readByte(address + 1) << 8;
	return  low | high;
}

/**
 *
 * <P>
 * Write a 16 bit Word to Memory.
 * </P>
 *
 */
void writeWord(int address, int value) {
	writeByte(address, value & 0xFF);
	writeByte(address + 1, value >> 8);
}


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
void accessMapper(int address, int value)
{

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
