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
int saveRAM[0x10000];

#define BANK_LENGTH 16
int programROMLenght;
int bank[BANK_LENGTH]; // Each bank is 0x1000
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
//                return apuRead(addr) | 0x40;
                return 40;
            }
//            return apuRead(addr) | 0x40;
            return 0;
        } else if (addr < 0x4018) {
            // High I/O Regs
            if (addr == 0x4016) {
                // Joypad #1
                return readJoyPadBit(0);
            } else if (addr == 0x4017) {
                return readJoyPadBit(1);
            }
            return 0;
        } else if (addr < 0x6000) {
            // Expansion ROM
            return accessMapperLowRead(addr);
        } else{
            return saveRAM[addr - 0x6000];
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
//                apuWrite(addr, value);
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
//                apuWrite(addr, value);
                return;
            case 0x4016: // Joypad #1
                if ((value & 0x1) == 0)
                    resetJoyPad();
                return;
            case 0x4017: // Joypad #2
                if ((value & 0x1) == 0)
                    resetJoyPad(1);
//                apuWrite(addr, value);
                return;
            }
            return;
        } else if (addr < 0x6000) {
            // Expansion ROM and Low Mapper Write Region
            accessMapperLow(addr, value);
        } else {
            accessMapper(addr, value);
            saveRAM[addr - 0x6000] = value;
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


void saveRAMState(FILE* file)
{
    fwrite(&RAM,sizeof(byte),0x0800, file);
    fwrite(&bank, sizeof(int), BANK_LENGTH, file);
    saveMapperState(file);
}

void loadRAMState(FILE* file)
{
    fread(&RAM,sizeof(byte),0x0800, file);
    fread(&bank, sizeof(int), BANK_LENGTH, file);
    loadMapperState(file);
}

