#include <stdlib.h>
#include "JoyPad.h"
/*
 * c
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
char joypad[2] = {0,0};
char joypadBit[2] = {0,0};

char* getJoyPadPoint() {
    return joypad;
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
int readJoyPadBit(int option){
    // Read the Indexed Bit
    int correction = joypad[option];
    if ((correction & 0x30) == 0x30) // Up and Down are pressed
        correction &= 0xCF;
    if ((correction & 0xC0) == 0xC0) // Left and Right are pressed
        correction &= 0x3F;
    int retVal = correction >> joypadBit[option];
    // Roll Index
    joypadBit[option] = (joypadBit[option] + 1) & 0x7;
    return retVal & 0x1;
}

void resetJoyPad(int option) {
    joypadBit[option] = 0;
}

void saveJoyPadState(FILE* file)
{
	fputc((int)joypad[0], file);
	fputc((int)joypad[1], file);
	fputc((int)joypadBit[0], file);
	fputc((int)joypadBit[1], file);
}
void loadJoyPadState(FILE* file)
{
	joypad[0] = fgetc(file);
	joypad[1] = fgetc(file);
	joypadBit[0] = fgetc(file);
	joypadBit[1] = fgetc(file);
}
