/*
 * JoyPad.h
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */

#ifndef JOYPAD_H_
#define JOYPAD_H_

#include <stdio.h>

char* getJoyPadPoint();
void resetJoyPad();
int readJoyPadBit(int option);
void saveJoyPadState(FILE* file);
void loadJoyPadState(FILE* file);
#endif /* JOYPAD_H_ */
