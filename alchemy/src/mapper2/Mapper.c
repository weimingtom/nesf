#include "Mapper.h"
#include "RAM.h"
#include "PPU.h"


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
void accessMapper(int addr, int data) {
    if (addr<0x8000) return;
    int romBankCount = getNum8KRomBanks();
    data &= romBankCount - 1;
    setCPUBanks(data*2, (data*2)+1, romBankCount - 2, romBankCount - 1);
}
/**
 *
 * <P>
 * Reset the Memory Mapper.
 * </P>
 *
 */
void resetMapper() {
    setCPUBanks(0,1,getNum8KRomBanks()-2,getNum8KRomBanks()-1);
    if (getNum1KVROMBanks() > 0)
          setPPUBanks(0,1,2,3,4,5,6,7);
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

void saveMapperState(FILE* file) {
}
void loadMapperState(FILE* file) {
}
