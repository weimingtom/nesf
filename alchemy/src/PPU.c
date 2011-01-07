/*
 * PPU.c
 *
 *  Created on: 2009-5-2
 *      Author: Administrator
 */
#include <string.h>
#include "NES.h"
#include "PPU.h"
#include "Mapper.h"
#include "screen.h"
/**
 *
 * <P>
 * Declare Buffer for pixels on current line.
 * </P>
 *
 */
byte linePalettes[34* 8];
int linePalettesSize = 272;
/**
 *
 * <P>
 * The Palette Memory (external to the PPU Memory)
 * </P>
 *
 */
int paletteMemory[0x20];
/**
 *
 * <P>
 * Array declaring if Sprites or Background have already been Plotted.
 * </P>
 *
 */
int solidBGPixel[35* 8 ];
int solidBGPixelSize = 280;
/**
 *
 * PPU Background Pattern Table Address
 *
 */
int bgPatternTableAddress = 0x0000;
/**
 *
 * PPU Sprite Pattern Table Address
 *
 */
int spPatternTableAddress = 0x1000;
/**
 *
 * Loopys PPU Refresh Register
 *
 */
int loopyV = 0;
/**
 *
 * Loopys PPU Temp Refresh Register
 *
 */
int loopyT = 0;
/**
 *
 * Loopys X Fine Scroll Register
 *
 */
int loopyX = 0;
/**
 *
 * The PPU Read Latch for Register 2007
 *
 */
int ppuReadLatch2007 = 0x00;
/**
 *
 * The general PPU Read Latch
 *
 */
int ppuLatch = 0x00;
/**
 *
 * Whether the current Frame is being Skipped
 *
 */
boolean skipIt = 0;
/**
 *
 * Whether the Sprites are currently visible
 *
 */
boolean showSprites = 1;
/**
 *
 * Whether the Background is currently visible
 *
 */
boolean showBackground = 1;
/**
 *
 * Sprite Memory
 *
 */
int spriteMemory[256];
/**
 *
 * Current index into Sprite Memory
 *
 */
int spriteMemoryAddress = 0;

boolean vram_write_protect;
/*************************************************************************************
 *
 * PPU Memory
 *
 ************************************************************************************/

int ppuMemory[0x4000];
int ppuBank[12];
int ppuAddressIncrement = 1;
int ppuVROMSize;
byte* ppuVROM;

int currentScanline = 0;
boolean latchMapper = 0;
boolean ppuAddressMode = 0;

int REG_2000 = 0x00;
int REG_2001 = 0x00;
int REG_2002 = 0x00;

int mirrorFourScreen = 0;
int mirrorHorizontal = 0;
int mirrorVertical = 0;

void setPPUlatchMapper(boolean v){
    latchMapper = v;
}
/**
 *
 * <P>
 * Blitz the Background on the specified scanline to internal buffer.
 * </P>
 *
 */
void backgroundBlitzer() {
    // Determine the X and Y Coordinates of the Current Offsets
    int tileX = (loopyV & 0x001F);
    int tileY = (loopyV & 0x03E0) >> 5;
    // Determine the Name Table Address
    int nameAddr = 0x2000 + (loopyV & 0x0FFF);
    // Determine the Attribute Table Address
    int attribAddr = 0x2000 + (loopyV & 0x0C00) + 0x03C0 + ((tileY & 0xFFFC) << 1) + (tileX >> 2);
    // Determine the Attribute Bits
    int attribBits = 0;
    if ((tileY & 0x0002) == 0) {
        if ((tileX & 0x0002) == 0)
            attribBits = (ppuRead(attribAddr) & 0x03) << 2;
        else
            attribBits = (ppuRead(attribAddr) & 0x0C);
    } else {
        if ((tileX & 0x0002) == 0)
            attribBits = (ppuRead(attribAddr) & 0x30) >> 2;
        else
            attribBits = (ppuRead(attribAddr) & 0xC0) >> 4;
    }
    // Declare Addresses for the Pattern Table Address and there Low and
    // High Values
    int patternAddr;
    int patternValueLo;
    int patternValueHi;
    // Calculate the X Offset into the Line
    int p = -loopyX;
    int solid = -loopyX;
    int i;
    for (i = 0; i < 33; i++) {
        // Grab Pattern Table Addresses
        patternAddr = bgPatternTableAddress + (ppuRead(nameAddr) << 4)
                + ((loopyV & 0x7000) >> 12);
        patternValueLo = ppuRead(patternAddr);
        patternValueHi = ppuRead(patternAddr + 8);
        // Latch Mapper on those Two Addresses
        if (latchMapper)
            mapperLatch(patternAddr);
        // Draw the Current Tile Data
        int patternMask;
        for (patternMask = 0x80; patternMask > 0; patternMask >>= 1) {
            // Grab the 2 Upper Bits of Colour
            int col = attribBits;
            // Grab the 2 Lower Bits of Colour
            if ((patternValueLo & patternMask) != 0)
                col |= 0x01;
            if ((patternValueHi & patternMask) != 0)
                col |= 0x02;
            // If Not Transparent then Draw and Mark as Drawn
            if ((col & 0x03) != 0) {
                if (solid > 0)
                    solidBGPixel[solid] = 0x01;
                if (showBackground && p > 0)
                    linePalettes[p] = col;
            } else {
                if (solid > 0)
                    solidBGPixel[solid] = 0x00;
                if (showBackground && p > 0)
                    linePalettes[p] = 0;
            }
            solid++;
            p++;
        }
        // Increment the Tile X Index and the Name Table Address
        tileX++;
        nameAddr++;
        // Check if we Crossed a Dual-Tile Boundary
        if ((tileX & 0x0001) == 0) {
            // Check if we Crossed a Quad-Tile Boundary
            if ((tileX & 0x0003) == 0) {
                // Check if we Crossed a Name Table Boundary
                if ((tileX & 0x001F) == 0) {
                    // Switch Name Tables
                    nameAddr ^= 0x0400;
                    attribAddr ^= 0x0400;
                    nameAddr -= 0x0020;
                    attribAddr -= 0x0008;
                    tileX -= 0x0020;
                }
                attribAddr++;
            }
            if ((tileY & 0x0002) == 0)
                if ((tileX & 0x0002) == 0)
                    attribBits = (ppuRead(attribAddr) & 0x03) << 2;
                else
                    attribBits = (ppuRead(attribAddr) & 0x0C);
            else if ((tileX & 0x0002) == 0)
                attribBits = (ppuRead(attribAddr) & 0x30) >> 2;
            else
                attribBits = (ppuRead(attribAddr) & 0xC0) >> 4;
        } // Dual-Tile Boundary Crossed
    } // Tiles Complete
    // Check for Clip and Mark as Not Painted (Entry 64)
    if ((REG_2001 & 0x02) == 0) {
        for (i = 0; i < 8; i++) {
            linePalettes[i] = 64;
            solidBGPixel[i] = 0;
        }
    }
}

/**
 *
 * <P>
 * Blitz the Sprites on the specified scanline to internal buffer.
 * </P>
 *
 */
void spriteBlitzer(int lineNum) {
    // Declare Sprite Coordinates
    int sprX;
    int sprY;
    // Declare In-Sprite Pixel Coordinates
    int x;
    int y;
    // Declare Start and End X Coordinates on Current Line of Sprite
    int xStart;
    int xEnd;
    // State that no Sprites are Currently on this Scanline
    int spritesInScanLine = 0;
    // Determine the Height of the Sprites (16 or 8 pixels)
    int sprHeight = ((REG_2000 & 0x20) != 0) ? 16 : 8;
    // Assume Less than 8 Sprites on Line Until Told Otherwise
    REG_2002 &= 0xDF;
    int s;
    for (s = 0; s < 64; s++) {
        // Determine the Lines the Sprite Crosses
        sprY = spriteMemory[s * 4] + 1;
        int lineOfSprite = lineNum - sprY;
        // Check for Intersection with Current Line
        if (lineOfSprite < 0 || lineOfSprite >= sprHeight)
            continue;
        // Report on Number of Sprites on Scanline (Shouldn't Draw > 8 but
        // will)
        if (++spritesInScanLine > 8) {
            REG_2002 |= 0x20;
        }
        // Grab the Immediate Attributes of the Sprite
        int tileIndex = spriteMemory[s * 4 + 1];
        int sprAttribs = spriteMemory[s * 4 + 2];
        sprX = spriteMemory[s * 4 + 3];
        // Grab the Extended Attributes of the Sprite
        boolean sprVFlip = (sprAttribs & 0x80) != 0;
        boolean sprHFlip = (sprAttribs & 0x40) != 0;
        boolean sprBGPriority = (sprAttribs & 0x20) != 0;
        // Assume That we are Drawing All 8 Horizontal Pixels of the Sprite
        xStart = 0;
        xEnd = 8;
        // Clip the Sprite if it Goes off to the Right
        if ((sprX + 7) > 255)
            xEnd -= ((sprX + 7) - 255);
        // Determine the Y Coordinate within the Sprite
        y = lineNum - sprY;
        // Calc offsets into Line Buffer and Solid Background Buffer
        int p = sprX + xStart;
        int solid = sprX + xStart;
        // Determine Direction to Read Sprite Data based on Horizontal
        // Flipping
        int incX = 1;
        // Check if Horizontally Flipped
        if (sprHFlip) {
            incX = -1;
            xStart = 7 - xStart;
            xEnd = 7 - xEnd;
        }
        // Check if Vertically Flipped
        if (sprVFlip)
            y = (sprHeight - 1) - y;
        // Latch Mapper with Tile Address (MMC2 Punchout needs this)
        if (latchMapper)
            mapperLatch(tileIndex << 4);
        // Loop Through Line in Sprite
        for (x = xStart; x != xEnd; x += incX) {
            // Declare Variables for Colour and Tile Information
            int col = 0x00;
            int tileAddr;
            int tileMask;
            // Don't Draw if a Higher Priority Sprite has Already Drawn
            if ((solidBGPixel[solid] & 2) == 0) {
                // Calculate the Tile Address
                if (sprHeight == 16) {
                    tileAddr = tileIndex << 4;
                    if ((tileIndex & 0x01) != 0) {
                        tileAddr += 0x1000;
                        if (y < 8)
                            tileAddr -= 16;
                    } else {
                        if (y >= 8)
                            tileAddr += 16;
                    }
                    tileAddr += y & 0x07;
                    tileMask = (0x80 >> (x & 0x07));
                } else {
                    tileAddr = tileIndex << 4;
                    tileAddr += y & 0x07;
                    tileAddr += spPatternTableAddress;
                    tileMask = (0x80 >> (x & 0x07));
                }
                // Determine the Two Lower Bits of Colour for Pixel
                if ((ppuRead(tileAddr) & tileMask) != 0)
                    col |= 0x01;
                tileAddr += 8;
                if ((ppuRead(tileAddr) & tileMask) != 0)
                    col |= 0x02;
                // Determine the Two Higher Bits of Colour for Pixel
                if ((sprAttribs & 0x02) != 0)
                    col |= 0x08;
                if ((sprAttribs & 0x01) != 0)
                    col |= 0x04;
                // Check if Sprite Not Transparent
                if ((col & 0x03) != 0) {
                    // Check if Sprite 0 was Hit and Background Written
                    if (s == 0 && solidBGPixel[solid] == 1) {
                        REG_2002 |= 0x40;
                        // if ((REG_2000 & 0x40) != 0) nes.cpu.cpuNMI();
                    }
                    // Check if Background Has Priority over Sprite
                    if ((REG_2001 & 0x04) != 0 || (p >= 8 && p < 248)) {
                        if (sprBGPriority) {
                            // Sprite Written
                            solidBGPixel[solid] |= 0x2;
                            // Actually Draw it if No Background is Drawn
                            // Over
                            if ((solidBGPixel[solid] & 1) == 0 && showSprites)
                                linePalettes[p] = 16 + col;
                        } else {
                            if (showSprites)
                                linePalettes[p] = 16 + col;
                            solidBGPixel[solid] |= 0x2; // Sprite Written
                        }
                    }
                }
            }
            // Increment Line Buffer Pointer
            p++;
            solid++;
        } // End of Loop Through Sprite X Coordinates
    }
    return;
}

/**
 *
 * <P>
 * Start a Vertical Blank.
 * </P>
 *
 */
void startVBlank() {
    REG_2002 |= 0x80;
}

/**
 *
 * <P>
 * End a Vertical Blank.
 * </P>
 *
 */
void endVBlank() {
    // Reset Vblank and Clear Sprite Hit
    REG_2002 &= 0x3F;
}

/**
 *
 * <P>
 * End a Frame.
 * </P>
 *
 */
void endFrame() {
}

/**
 *
 * <P>
 * Check if NMI is enabled.
 * </P>
 *
 */
boolean nmiEnabled() {
    return (REG_2000 & 0x80) != 0;
}

/**
 *
 * <P>
 * Draw the next Scanline.
 * </P>
 *
 */
void drawScanLine() {
    int i;
    // Clear the Line Buffer
//	memset(linePalettes, 64, sizeof(byte)*linePalettesSize);;
    boolean skipIt = false;
    if ((REG_2001 & 0x18) != 0x00) {
        loopyScanlineStart();
        skipIt = renderLine();
        loopyScanlineEnd();
    } else {
        skipIt = renderLine();
    }
    currentScanline++;
    // If Not a Skipped Frame then Send Pixels to TV Controller
    if (!skipIt) {
        // Convert Buffered NES 4-bit Colours into 32-Bit Palette Entries
        for (i = 0; i < linePalettesSize; i++) {
            if (linePalettes[i] != 64)
                linePalettes[i] = paletteMemory[linePalettes[i]] & 63;
        }
        // Draw to Screen (using TV Controller)
        setPixels(linePalettes, linePalettesSize);
    }
}

/**
 *
 * <P>
 * Prepare the Refresh Data Address for the start of a new Frame.
 * </P>
 *
 * This information was provided in a document called
 * "The Skinny on NES Scrolling" by Loopy on the NESDEV eGroup.
 *
 */
void startFrame() {
    // Set the Scanline Number
    currentScanline = 0;
    // Check either Background or Sprites are Displayed
    if ((REG_2001 & 0x18) != 0x00)
        loopyV = loopyT;
}

/**
 *
 * <P>
 * Prepare the Refresh Data Address for the end of the scanline.
 * </P>
 *
 * This information was provided in a document called
 * "The Skinny on NES Scrolling" by Loopy on the NESDEV eGroup.
 *
 */
void loopyScanlineEnd() {
    // Check if Subtile Y Offset is 7
    if ((loopyV & 0x7000) == 0x7000) {
        // Set SubTile Y Offset = 0
        loopyV &= 0x8FFF;
        // Check if Name Table Line = 29
        if ((loopyV & 0x03E0) == 0x03A0) {
            // Switch Name Tables
            loopyV ^= 0x0800;
            // Name Table Line 0
            loopyV &= 0xFC1F;
        } else {
            // Check if Line = 31
            if ((loopyV & 0x03E0) == 0x03E0) {
                // Name Table Line = 0
                loopyV &= 0xFC1F;
            } else {
                // Increment the Table Line Number
                loopyV += 0x0020;
            }
        }
    } else {
        // Next Subtile Y Offset
        loopyV += 0x1000;
    }
}

/**
 *
 * <P>
 * Prepare the Refresh Data Address for the start of a new Scanline.
 * </P>
 *
 * This information was provided in a document called
 * "The Skinny on NES Scrolling" by Loopy on the NESDEV eGroup.
 *
 */
void loopyScanlineStart() {
    loopyV &= 0xFBE0;
    loopyV |= loopyT & 0x041F;
}

/**
 *
 * <P>
 * Read from a specified register on the PPU.
 * </P>
 *
 */
int readFromPPU(int address) {
    // Determine the Address
    switch (address & 0x7) {
    case 0x2: // PPU Status Register $2002
        // Reset the PPU Addressing Mode Toggle
        reset2005Toggle();
        // Clear vBlank Flag
        int j = REG_2002;
        REG_2002 &= 0x7F;
        // Return Bits 7-4 of Unmodified Register 2002 with Bits 3-0 of the
        // Latch
        return (j & 0xE0) | (ppuLatch & 0x1F);
    case 0x4: // SPR_RAM I/O Register (RW)
        spriteMemoryAddress++;
        spriteMemoryAddress &= 0xFF;
        return spriteMemory[spriteMemoryAddress];
    case 0x7: // VRAM I/O Register $2007
        // Return the PPU Latch Value and Read a new Value from PPU Memory
        // into the Latch
        ppuLatch = ppuReadLatch2007;
        ppuReadLatch2007 = ppuRead(-1);
        return ppuLatch;
    default: // Return PPU Latch
        return ppuLatch & 0xFF;
    }
}

/**
 *
 * <P>
 * Render the specified line to the TV Controller.
 * </P>
 *
 */
boolean renderLine() {
    // Set the current Scanline
    skipIt = setScanLineNum(currentScanline);
    // Buffer the Background
    if ((REG_2001 & 0x08) != 0x00) {
        backgroundBlitzer();
    }
    // Buffer the Sprites
    if ((REG_2001 & 0x10) != 0x00) {
        spriteBlitzer(currentScanline);
    }
    return skipIt;
}

/**
 *
 * <P>
 * Write to a specified register on the PPU.
 * </P>
 *
 */
void writeToPPU(int address, int value) {
    // Set the PPU Latch
    ppuLatch = value & 0xFF;
    // Calculate the Address (8 Registers Mirrored)
    address = (address & 0x7) + 0x2000;
    // Determine the Address being Written to
    switch (address & 7) {
    case 0x0: // PPU Control Register #1
        // Set the Register Value
        REG_2000 = value;
        // Set the Background and Sprite Pattern Table Addresses
        bgPatternTableAddress = ((value & 0x10) != 0) ? 0x1000 : 0x0000;
        spPatternTableAddress = ((value & 0x08) != 0) ? 0x1000 : 0x0000;
        // Set the Address Increment Value for the PPU
        ppuAddressIncrement = ((value & 0x4) != 0) ? 32 : 1;
        // Change the Temporary Refresh Address for the PPU
        loopyT &= 0xF3FF;
        loopyT |= (value & 3) << 10;
        return;
    case 0x1: // PPU Control Register #2
        REG_2001 = value;
        return;
    case 0x2: // Status Register (Cannot be Written to)
        return;
    case 0x3: // SPR-RAM Address Register $2003
        spriteMemoryAddress = value;
        return;
    case 0x4: // SPR-RAM I/O Register $2004
        spriteMemory[spriteMemoryAddress] = (value & 0xFF);
        spriteMemoryAddress++;
        spriteMemoryAddress &= 0xFF;
        return;
    case 0x5: // VRAM Address Register #1
        set2005(value);
        return;
    case 0x6: // VRAM Address Register #2
        set2006(value);
        return;
    case 0x7: // VRAM I/O Register $2007
        ppuWrite(value);
        return;
    }
}

// PPUMemory Functions
/**
 *
 * <P>
 * Initialises the PPU Memory.
 * </P>
 *
 * @param vrom
 *            The Cartridges VROM.
 * @param verticalMirroring
 *            True if Cartridge uses Vertical mirroring.
 * @param fourScreenMirror
 *            True if Cartridge uses Four Screen Mirroring.
 *
 */
void initPPU(byte* rom, int size, boolean vMirroring, boolean fourScreenNT) {
    // Reset the PPU
    ppuVROM = rom;
    ppuVROMSize = size;
    vram_write_protect = (size != 0);

    memset(ppuMemory,0,sizeof(int)*0x4000);
    memset(spriteMemory,0,sizeof(int)*256);
    // Set the FourScreen Name Table Mirroring Method
    mirrorFourScreen = fourScreenNT;
    mirrorVertical = vMirroring;
    mirrorHorizontal = !vMirroring;

    REG_2000 = 0x00;
    REG_2001 = 0x00;
    REG_2002 = 0x00;
    // Clear the PPU Memory

    // Initialise the 1K PPU Memory Banks at Correct Offsets into VROM
    ppuBank[0x0] = 0x0000;
    ppuBank[0x1] = 0x0400;
    ppuBank[0x2] = 0x0800;
    ppuBank[0x3] = 0x0C00;
    ppuBank[0x4] = 0x1000;
    ppuBank[0x5] = 0x1400;
    ppuBank[0x6] = 0x1800;
    ppuBank[0x7] = 0x1C00;
    ppuBank[0x8] = 0x2000;
    ppuBank[0x9] = 0x2000;
    ppuBank[0xA] = 0x2000;
    ppuBank[0xB] = 0x2000;
    // Configure the Mirroring Method
    setMirror();
    // Set the Cartridge VROM
    // Clear Sprite RAM
    spriteMemoryAddress = 0x00;
    // Clear Pattern Table Addresses
    bgPatternTableAddress = 0x0000;
    spPatternTableAddress = 0x0000;
    // Reset the VRAM Address Registers
    currentScanline = 0;
    reset2005Toggle();
    loopyV = 0x00;
    loopyT = 0x00;
    loopyX = 0x00;
    // Reset the PPU Latch
    ppuReadLatch2007 = 0x00;
    ppuLatch = 0x00;
    ppuAddressIncrement = 1;
    // Reset Palette
    latchMapper = 0;
}

/**
 *
 * <P>
 * Read from PPU Memory at a specified address.
 * </P>
 *
 * @return The value at the specified cell of PPU Memory.
 *
 */
int ppuRead(int addr) {
    if (addr == -1) {
        // Determine the Address to Read from
        int addr = loopyV;
        // Increment and Wrap the PPU Address Register
        loopyV += ppuAddressIncrement;
        addr &= 0x3FFF;
        // Call the Read Function
        return ppuRead(addr);
    }
    if (addr < 0x2000)
        return readPatternTable(addr);
    if (addr >= 0x3000) {
        if (addr >= 0x3F00) {
            // Palette Read
            return paletteMemory[addr & 0x1F];
        }
        addr &= 0xEFFF;
    }
    return ppuVRAM(addr);
}

/**
 *
 * <P>
 * Read value from Pattern Table.
 * </P>
 *
 * @param address
 *            Address to read from in Pattern Table.
 *
 * @return The value at the specifed address.
 *
 */
int readPatternTable(int addr) {
    // Ensure Range of Address
    addr &= 0x1FFF;
    // Check for VROM Banks
    return ppuVROMSize != 0 ? ppuVROM[ppuBank[addr >> 10] + (addr & 0x3FF)]
                             :ppuMemory[ppuBank[addr >> 10] + (addr & 0x3FF)];
}

/**
 *
 * <P>
 * Resets PPU Register 0x2005.
 * <P>
 *
 */
void reset2005Toggle() {
    ppuAddressMode = false;
}

/**
 *
 * <P>
 * Sets VRAM Address Register 1.
 * </P>
 *
 * @param value
 *            The byte written to 0x2005.
 *
 */
void set2005(int value) {
    // The next use of this function will determine the other byte
    ppuAddressMode = !ppuAddressMode;
    // Set the Corresponding Byte of the Address
    if (ppuAddressMode) {
        // First Write : Horizontal Scroll
        loopyT &= 0xFFE0;
        loopyT |= (value & 0xF8) >> 3;
        loopyX = value & 0x07;
    } else {
        // Second Write : Vertical Scroll
        loopyT &= 0xFC1F;
        loopyT |= (value & 0xF8) << 2;
        loopyT &= 0x8FFF;
        loopyT |= (value & 0x07) << 12;
    }
}

/**
 *
 * <P>
 * Sets VRAM Address Register 2.
 * </P>
 *
 * Setting this address requires two writes to this function, the first
 * write will set the most significant byte of the address and the second
 * write will set the least significant byte of the address.
 *
 * @param value
 *            The byte written to 0x2006.
 *
 */
void set2006(int value) {
    // The next use of this function will determine the other byte
    ppuAddressMode = !ppuAddressMode;
    // Set the corresponding Byte of the address
    if (ppuAddressMode) {
        // First Write
        loopyT &= 0x00FF;
        loopyT |= (value & 0x3F) << 8;
    } else {
        // Second Write
        loopyT &= 0xFF00;
        loopyT |= value;
        loopyV = loopyT;
    }
}

/**
 *
 * <P>
 * Sets the Mirroring Mode for PPU Memory.
 * </P>
 *
 */
void setMirror() {
    // Set Four Screen Mirroring
    if (mirrorFourScreen)
        setPPUMirroring(0, 1, 2, 3);
    // Set Horizontal Mirroring
    else if (mirrorHorizontal)
        setPPUMirroring(0, 0, 1, 1);
    // Set Vertical Mirroring
    else if (mirrorVertical)
        setPPUMirroring(0, 1, 0, 1);
    // If no Mirroring is Known then Select Four Screen
    else
        setPPUMirroring(0, 1, 2, 3);
}

/**
 *
 * <P>
 * Sets the Name Table bank numbers for use in Mirroring.
 * </P>
 *
 * Each Nametable bank must be within the range of 0-3
 *
 */
void setPPUMirroring(int nt0, int nt1, int nt2, int nt3) {
    // Ensure within Correct Range
    nt0 &= 0x3;
    nt1 &= 0x3;
    nt2 &= 0x3;
    nt3 &= 0x3;
    // Set the Bank Offsets
    ppuBank[0x8] = 0x2000 + (nt0 << 10);
    ppuBank[0x9] = 0x2000 + (nt1 << 10);
    ppuBank[0xA] = 0x2000 + (nt2 << 10);
    ppuBank[0xB] = 0x2000 + (nt3 << 10);
}

/**
 *
 * <P>
 * Write value to currently indexed cell of PPU Memory.
 * </P>
 *
 * @param value
 *            The value to be written.
 *
 */
void ppuWrite(int value) {
    // Determine the PPU Address
    int addr = loopyV;
    // Increment the PPU Address
    loopyV += ppuAddressIncrement;
    addr &= 0x3FFF;
    // Determine the Address Range
    if (addr >= 0x3000) {
        // Deal with Palette Writes
        if (addr >= 0x3F00) {
            // Palette Entry
            if ((addr & 0x000F) == 0x0000) {
                paletteMemory[0x00] = (value & 0x3F);
                paletteMemory[0x10] = (value & 0x3F);
            } else {
                paletteMemory[addr & 0x001F] = (value & 0x3F);
            }
            return;
        }
        // Mirror 0x3000 to 0x2000
        addr &= 0xEFFF;
    }
    // Write to VRAM
    if (!(vram_write_protect && addr < 0x2000)) {
        ppuMemory[ppuBank[addr >> 10] + (addr & 0x3FF)] = value;
    } else {
    }
}

/**
 *
 * <P>
 * Returns a value from VRAM.
 * </P>
 *
 */
int ppuVRAM(int addr) {
    return ppuMemory[ppuBank[addr >> 10] + (addr & 0x3FF)];
}

/**
 *
 * <P>
 * Sets the offset in VROM of a PPU Memory Bank.
 * </P>
 *
 * @param bankNum
 *            The bank number to configure (0-7).
 * @param offsetInCHRROM
 *            The offset in VROM that the 1K bank starts at.
 *
 */
void setPPUBankStartAddress(int bankNum, int offsetInVROM) {
    // Validate
    int num1Kvrombanks = ppuVROMSize >> 10;
    if (bankNum >= num1Kvrombanks)
        return;
    if (offsetInVROM > ppuVROMSize)
        return;
    // Set the Bank Start address
    ppuBank[bankNum] = offsetInVROM;
}

/**
 *
 * <P>
 * Set VRAM Bank
 * </P>
 *
 */
void setPPUVRAMBank(int bank, int banknum) {
    if (bank < 8) {
        // Map to Start of Pattern Tables
        ppuBank[bank] = 0x0000 + ((banknum & 0x0F) << 10);
    } else if (bank < 12) {
        // Map to Start of Name Tables
        ppuBank[bank] = 0x2000 + ((banknum & 0x03) << 10);
    }
}

void loadPPUState(FILE* file)
{
    fread(&ppuMemory, sizeof(int), 0x4000, file);
    fread(&spriteMemory, sizeof(int), 256, file);
    fread(&paletteMemory, sizeof(int), 0x20, file);
    fread(&ppuBank, sizeof(int), 12, file);

    REG_2000 = fgetc(file)& 0xFF;
    REG_2001 = fgetc(file)& 0xFF;
    REG_2002 = fgetc(file)& 0xFF;

    bgPatternTableAddress = fgetc(file) + (fgetc(file) << 8);
    spPatternTableAddress = fgetc(file) + (fgetc(file) << 8);
 // Save Loopy Registers
    loopyV = fgetc(file) +
            (fgetc(file) << 8) +
            (fgetc(file) << 0x10) +
            (fgetc(file) << 0x18);
    loopyT = fgetc(file) +
            (fgetc(file) << 8) +
            (fgetc(file) << 0x10) +
            (fgetc(file) << 0x18);
    loopyX = fgetc(file);
    ppuReadLatch2007 = fgetc(file);
    ppuLatch = fgetc(file);

    ppuAddressMode = fgetc(file);
    mirrorFourScreen = fgetc(file);
    mirrorHorizontal = fgetc(file);
    mirrorVertical = fgetc(file);

    ppuAddressIncrement = fgetc(file);
    spriteMemoryAddress = fgetc(file);
}

void savePPUState(FILE* file)
{
    fwrite(&ppuMemory, sizeof(int), 0x4000, file);
    fwrite(&spriteMemory, sizeof(int), 256, file);
    fwrite(&paletteMemory, sizeof(int), 0x20, file);
    fwrite(&ppuBank, sizeof(int), 12, file);

    fputc(REG_2000 & 0xFF, file);
    fputc(REG_2001 & 0xFF, file);
    fputc(REG_2002 & 0xFF, file);

    fputc((bgPatternTableAddress >> 0x00) & 0xFF, file);
    fputc((bgPatternTableAddress >> 0x08) & 0xFF, file);
    fputc((spPatternTableAddress >> 0x00) & 0xFF, file);
    fputc((spPatternTableAddress >> 0x08) & 0xFF, file);
 // Save Loopy Registers
    fputc((loopyV >> 0x00) & 0xFF, file);
    fputc((loopyV >> 0x08) & 0xFF, file);
    fputc((loopyV >> 0x10) & 0xFF, file);
    fputc((loopyV >> 0x18) & 0xFF, file);
    fputc((loopyT >> 0x00) & 0xFF, file);
    fputc((loopyT >> 0x08) & 0xFF, file);
    fputc((loopyT >> 0x10) & 0xFF, file);
    fputc((loopyT >> 0x18) & 0xFF, file);
    fputc(loopyX & 0xFF, file);
    fputc(ppuReadLatch2007 & 0xFF, file);
    fputc(ppuLatch & 0xFF, file);

    fputc(ppuAddressMode   ? 0xFF : 0x00, file);
    fputc(mirrorFourScreen ? 0xFF : 0x00, file);
    fputc(mirrorHorizontal ? 0xFF : 0x00, file);
    fputc(mirrorVertical   ? 0xFF : 0x00, file);

    fputc(ppuAddressIncrement & 0xFF, file);
    fputc(spriteMemoryAddress & 0xFF, file);
}
