#include <stdlib.h>
#include "JoyPad.h"
/*
 * c
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
char joypad;
/**
 *
 * Declare variable for current button being read
 *
 */
int joypadBit = 0;

char* getJoyPadPoint() {
	return &joypad;
}
/**
 *
 * <P>
 * Read value from the JoyPad for the currently indexed button.
 * </P>
 *
 * @return Returns 1 if current indexed button is pressed, else 0.
 *
 */
int readJoyPadBit() {
	// Read the Indexed Bit

	int correction = joypad;
	if ((correction & 0x30) == 0x30) // Up and Down are pressed
		correction &= 0xCF;
	if ((correction & 0xC0) == 0xC0) // Left and Right are pressed
		correction &= 0x3F;
	int retVal = correction >> joypadBit;
	// Roll Index
	joypadBit = (joypadBit + 1) & 0x7;
	return retVal & 0x1;
}

/**
 *
 * <P>
 * Reset the JoyPad so that the next read is for button A.
 * </P>
 *
 */
void resetJoyPad() {
	joypadBit = 0;
}
