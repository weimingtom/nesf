/*
 * screen.h
 *
 *  Created on: 2009-5-11
 *      Author: Administrator
 */

#ifndef SCREEN_H_
#define SCREEN_H_
#include "NES.h"

boolean setScanLineNum(int sl);

/**
 *
 * <P>
 * Sets the frame skip for the TV Controller.
 * </P>
 *
 * @param value
 *            The number of frames to skip before drawing.
 *
 */
void setFrameSkip(int value);
/**
 *
 * <P>
 * Sets an Array of Pixels representing the current Scanline.
 * </P>
 *
 */
void setPixels(byte* palEntries, int lineNum);

int* createScreen();

#endif /* SCREEN_H_ */
