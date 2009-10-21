/*
 * NESCafe.c
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NES.h"
#include "PPU.h"
#include "Mapper.h"
#include "M6502.h"
#include "RAM.h"
#include "screen.h"

#define CYCLES_PER_LINE 116.0f

int videoBuffer[256* 240 ];
int frameIRQEnabled;

void emulateFrame() {
	int i;
	// Start PPU Frame
	startFrame();
	// Lines 0-239
	for (i = 0; i < 240; i++) {
		emulateCPUCycles(CYCLES_PER_LINE);
		if (syncH(i) != 0)
			IRQ();
		drawScanLine();
	}
	// Close PPU Frame
	endFrame();
	// Frame IRQ
	if ((frameIRQEnabled & 0xC0) == 0) {
		IRQ();
	}
	// Lines 240-261
	for (i = 240; i <= 261; i++) {
		// End of Virtual Blank
		if (i == 261) {
			endVBlank();
		}
		// Start of Virtual Blank
		if (i == 241) {
			startVBlank();
			syncV();
			emulateCPUCycles(1);
			if (nmiEnabled())
				NMI();
			emulateCPUCycles(CYCLES_PER_LINE - 1);
			if (syncH(i) != 0)
				IRQ();
			continue;
		}
		emulateCPUCycles(CYCLES_PER_LINE);
		if (syncH(i) != 0)
			IRQ();
	}
}

void cpuLoadRom(char* data) {

	typedef struct {
		char ID[4];
		byte numProgramROMBanks;
		byte numCharacterROMBanks;
		byte lowMapper;
		byte highMapper;
		char reserve[8];
	} iNES_HEADER;

	iNES_HEADER head;

	int headSize;
	headSize = sizeof(head);

	memcpy(&head, data, headSize);
	data += headSize;

	if (memcmp(head.ID, "NES\x1a", 4)) {
		return;
	}

	boolean vMirroring = (head.lowMapper & 0x1) != 0;
//	boolean hasSaveRAM = (head.lowMapper & 0x2) != 0;
//	boolean hasTrainer = (head.lowMapper & 0x4) != 0;
	boolean fourScreenNT = (head.lowMapper & 0x8) != 0;

	int programROMSize = head.numProgramROMBanks * 0x4000;
	int charROMSize = head.numCharacterROMBanks * 0x2000;

	byte* progROM = malloc( programROMSize );
	byte* charROM = malloc( charROMSize );

	memcpy(progROM, data, programROMSize);
	data += programROMSize;

	memcpy(charROM, data, charROMSize);
	data += charROMSize;

	//mapper
//	if (memcmp(head.reserve, reserve, 8) == 0)
//		cart.MMC_type |= (head.ROM_type2 & 0xF0);
	// Check the Signature is Correct (NES)
	// Read the Number of 16k Banks of Program ROM
	// Load the Memory with the Cartridge Image
	initRAM(progROM, programROMSize);
	initPPU(charROM, charROMSize, vMirroring, fourScreenNT);
	resetMapper();
//	setCRC(cart.crc32);
	initResetCPU();
}


int* connectScreen()
{
	return videoBuffer;
}

#ifdef DEBUG_NES
void loadCart() {
	// Create Input Stream
	//	NESCart cart;
	FILE* fp = fopen("f:\\vNEs.nes", "rb");
	fseek(fp, 0L, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	char* data = malloc(len);
	fread(data, sizeof(char), len, fp);
	cpuLoadRom(data);
	free(data);
}

int main() {
	loadCart();
	while (1) {
		emulateFrame();
	}
	return 0;
}
#endif
