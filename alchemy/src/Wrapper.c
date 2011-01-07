/*
 * Wrapper.c
 *
 *  Created on: 2009-5-11
 *      Author: Administrator
 */

#include "NES.h"
#include "JoyPad.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define DEBUG_NES

#ifndef DEBUG_NES
#include <AS3.h>

char* as3_cart;

AS3_Val connectFCScreen(void* self, AS3_Val args)
{
    int* screen = connectScreen() ;
    return AS3_Ptr( screen );
}

AS3_Val simulateFrame(void* self, AS3_Val args)
{
    emulateFrame();
    return 0;
}

AS3_Val initCartMem(void* self, AS3_Val args)
{
    FILE* fp = fopen("rom", "rb");
    fseek(fp, 0L, SEEK_END);
    int len = ftell(fp);
    rewind(fp);
    char* data = malloc(len);
    fread(data, sizeof(char), len, fp);
    cpuLoadRom(data);
    free(data);
    fclose(fp);
    return 0;
}

AS3_Val connectJoyPad(void* self, AS3_Val args)
{
    return AS3_Ptr(getJoyPadPoint() );
}

static int readByteArray(void *cookie, char *dst, int size)
{
    return AS3_ByteArray_readBytes(dst, (AS3_Val)cookie, size);
}

/* Does a FILE * write against a ByteArray */
static int writeByteArray(void *cookie, const char *src, int size)
{
    return AS3_ByteArray_writeBytes((AS3_Val)cookie, (char *)src, size);
}

/* Does a FILE * lseek against a ByteArray */
static fpos_t seekByteArray(void *cookie, fpos_t offs, int whence)
{
    return AS3_ByteArray_seek((AS3_Val)cookie, offs, whence);
}

/* Does a FILE * close against a ByteArray */
static int closeByteArray(void * cookie)
{
    AS3_Val zero = AS3_Int(0);

    /* just reset the position */
    AS3_SetS((AS3_Val)cookie, "position", zero);
    AS3_Release(zero);
    return 0;
}

AS3_Val saveState(void* self, AS3_Val args)
{
    void * dest;
    AS3_ArrayValue(args, "AS3ValType", &dest);

    FILE* fp = funopen((void *)dest, readByteArray, writeByteArray, seekByteArray, closeByteArray);
    fseek(fp, 0L, SEEK_END);
    int len = ftell(fp);
    rewind(fp);
    saveCPUState(fp);
    savePPUState(fp);
    saveRAMState(fp);
    saveJoyPadState(fp);
    fclose(fp);
    return 0;
}

AS3_Val loadState(void* self, AS3_Val args)
{
    void * dest;
    AS3_ArrayValue(args, "AS3ValType", &dest);

    FILE* fp = funopen((void *)dest, readByteArray, writeByteArray, seekByteArray, closeByteArray);
    fseek(fp, 0L, SEEK_END);
    int len = ftell(fp);
    rewind(fp);
    loadCPUState(fp);
    loadPPUState(fp);
    loadRAMState(fp);
    loadJoyPadState(fp);
    fclose(fp);
    return 0;
}

short* buf = 0;

AS3_Val playSound(void* self, AS3_Val args)
{
    AS3_Val byteArray = AS3_Undefined();
    AS3_ArrayValue(args, "AS3ValType,"
                         "IntType",
                         &byteArray);

    /* Fill sample buffer */
    playAPUSound(buf, 16384);
    AS3_ByteArray_writeBytes(byteArray, buf, 32768);
//    AS3_Release( byteArray );
    return 0;
}
#define AS3_METHOD "\
connectFCScreen: AS3ValType,\
simulateFrame: AS3ValType,\
initCartMem: AS3ValType,\
connectJoyPad: AS3ValType,\
saveState: AS3ValType,\
loadState: AS3ValType,\
playSound: AS3ValType"


int main()
{
	buf = malloc(32768);
    AS3_Val connectFCScreen_Method =
        AS3_Function( NULL, connectFCScreen );

    AS3_Val simulateFrame_Method =
        AS3_FunctionAsync( NULL, simulateFrame );

    AS3_Val initCartMem_Method =
        AS3_Function( NULL, initCartMem );

    AS3_Val connectJoyPad_Method =
        AS3_Function( NULL, connectJoyPad );

    AS3_Val saveState_Method =
        AS3_Function( NULL, saveState );

    AS3_Val loadState_Method =
        AS3_Function( NULL, loadState );

    AS3_Val playSound_Method =
        AS3_Function( NULL, playSound );

    AS3_Val result = AS3_Object(AS3_METHOD,
                                connectFCScreen_Method,
                                simulateFrame_Method,
                                initCartMem_Method,
                                connectJoyPad_Method,
                                saveState_Method,
                                loadState_Method,
                                playSound_Method);
    AS3_Release( connectFCScreen_Method );
    AS3_Release( simulateFrame_Method );
    AS3_Release( initCartMem_Method );
    AS3_Release( connectJoyPad_Method );
    AS3_Release( saveState_Method );
    AS3_Release( loadState_Method );
    AS3_Release( playSound_Method );
    AS3_LibInit( result );
    return 0;
}

#endif
