/*
 * Wrapper.c
 *
 *  Created on: 2009-5-11
 *      Author: Administrator
 */

#include "NES.h"
#include "JoyPad.h"

#define DEBUG_NES

#ifndef DEBUG_NES
#include "AS3.h"

char* as3_cart;

AS3_Val connectFCScreen(void* self, AS3_Val args)
{
	int* screen = connectScreen();
	return AS3_Ptr( screen );
}

AS3_Val simulateFrame(void* self, AS3_Val args)
{
	emulateFrame();
	return 0;
}

AS3_Val initCartMem(void* self, AS3_Val args)
{
	int cartSize;
	AS3_ArrayValue(args, "IntType", &cartSize);
	as3_cart = malloc( cartSize * sizeof(int) );
	return AS3_Ptr( as3_cart );
}

AS3_Val noticeLoadCart( void* self, AS3_Val args )
{
	cpuLoadRom(as3_cart);
	return 0;
}

AS3_Val connectJoyPad(void* self, AS3_Val args)
{
	return AS3_Ptr( getJoyPadPoint() );
}

#define AS3_METHOD "\
connectFCScreen: AS3ValType,\
simulateFrame: AS3ValType,\
initCartMem: AS3ValType,\
noticeLoadCart: AS3ValType,\
connectJoyPad: AS3ValType"


int main()
{
	AS3_Val connectFCScreen_Method =
		AS3_Function( NULL, connectFCScreen );

	AS3_Val simulateFrame_Method =
		AS3_Function( NULL, simulateFrame );

	AS3_Val initCartMem_Method =
		AS3_Function( NULL, initCartMem );

	AS3_Val noticeLoadCart_Method =
		AS3_Function( NULL, noticeLoadCart );

	AS3_Val connectJoyPad_Method =
		AS3_Function( NULL, connectJoyPad );

	AS3_Val result = AS3_Object(AS3_METHOD,
								connectFCScreen_Method,
								simulateFrame_Method,
								initCartMem_Method,
								noticeLoadCart_Method,
								connectJoyPad_Method);
	AS3_Release( connectFCScreen_Method );
	AS3_Release( simulateFrame_Method );
	AS3_Release( initCartMem_Method );
	AS3_Release( noticeLoadCart_Method );
	AS3_Release( connectJoyPad_Method );
	AS3_LibInit( result );
	return 0;
}

#endif
