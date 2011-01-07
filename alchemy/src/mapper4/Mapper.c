
#include "Mapper.h"
#include "RAM.h"
#include "PPU.h"

int irq_counter=0;
int irq_enabled=0;
int irq_latch=0;
int regs[8];
int prg0 = 0;
int prg1 = 1;
int chr01 = 0;
int chr23 = 0;
int chr4 = 0;
int chr5 = 0;
int chr6 = 0;
int chr7 = 0;

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
void accessMapper(int addr, int data)
{
    if (addr < 0x8000) return;
// Determine the Function
   switch(addr & 0xE001)
   {
      // Command Register
         case 0x8000:
            {
               regs[0] = data;
               MMC3_set_PPU_banks();
               MMC3_set_CPU_banks();
            }
            break;
      // Activate Register
         case 0x8001:
            {
               regs[1] = data;
               int bank_num = regs[1];
               switch(regs[0] & 0x07)
               {
                  case 0x00:
                     {
                        bank_num &= 0xfe;
                        chr01 = bank_num;
                        MMC3_set_PPU_banks();
                     }
                     break;
                  case 0x01:
                     {
                        bank_num &= 0xfe;
                        chr23 = bank_num;
                        MMC3_set_PPU_banks();
                     }
                     break;
                  case 0x02:
                     {
                        chr4 = bank_num;
                        MMC3_set_PPU_banks();
                     }
                     break;
                  case 0x03:
                     {
                        chr5 = bank_num;
                        MMC3_set_PPU_banks();
                     }
                     break;
                  case 0x04:
                     {
                        chr6 = bank_num;
                        MMC3_set_PPU_banks();
                     }
                     break;
                  case 0x05:
                     {
                        chr7 = bank_num;
                        MMC3_set_PPU_banks();
                     }
                     break;
                  case 0x06:
                     {
                        prg0 = bank_num;
                        MMC3_set_CPU_banks();
                     }
                     break;
                  case 0x07:
                     {
                        prg1 = bank_num;
                        MMC3_set_CPU_banks();
                     }
                     break;
               }
            }
            break;
      // Handle Mirroring
         case 0xA000:
            {
               regs[2] = data;
               if (!mirrorFourScreen)
               {
                  if ((data & 0x01)!=0)
                  {
                      setPPUMirroring(0, 0, 1, 1);
                  }
                  else
                  {
                      setPPUMirroring(0, 1, 0, 1);
                  }
               }
            }
            break;
      // Handle Save RAM at 0x6000 - 0x7FFF
         case 0xA001:
            {
               regs[3] = data;
//               enableSaveRAM = ((data & 0x80) == 0x80);
            }
            break;
      // Store IRQ Counter
         case 0xC000:
            {
               regs[4] = data;
               irq_counter = regs[4];
            }
            break;
      // Store IRQ Counter
         case 0xC001:
            {
               regs[5] = data;
               irq_latch = regs[5];
            }
            break;
      // Disable IRQ
         case 0xE000:
            {
               regs[6] = data;
               irq_enabled = false;
            }
            break;
      // Enable IRQ
         case 0xE001:
            {
               regs[7] = data;
               irq_enabled = true;
            }
            break;
   }
}

void accessMapperLow(int address, int value) {
}

int accessMapperLowRead(int addr) {
    return (addr >> 8) & 0xFF;
}
/**
 *
 * <P>
 * Latch the Memory Mapper.
 * </P>
 *
 * @param data
 *            A Mapper-specific data value that is passed to the Latch.
 *
 */
void mapperLatch(int data) {
}

/**
 *
 * <P>
 * Reset the Memory Mapper.
 * </P>
 *
 */
void resetMapper() {

    // Clear the Registers
    int i = 0;
    for(i=0; i < 8; i++) regs[i] = 0x00;
    // Switch Banks 8-B to First 16k of Program ROM
    prg0 = 0;
    prg1 = 1;
    MMC3_set_CPU_banks();
    // Set VROM Banks
    if (getNum1KVROMBanks()>0)
    {
       chr01 = 0;
       chr23 = 2;
       chr4  = 4;
       chr5  = 5;
       chr6  = 6;
       chr7  = 7;
       MMC3_set_PPU_banks();
    }
    else
    {
       chr01 = chr23 = chr4 = chr5 = chr6 = chr7 = 0;
    }
    // Reset IRQ Status
    irq_enabled = false;
    irq_counter = 0;
    irq_latch = 0;
    // Switch PPU Memory Banks
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
    if (irq_enabled)
                {
       // Check for Visible Scanline
          if ((scanline >= 0) && (scanline < 240))
          {
             // Check if Background or Sprites Enabled
                if ((REG_2001 & 0x18) != 00)
                {
                   // Decrement IRQ Counter
                      irq_counter--;
                   // Check if Counter Down to Zero
                      if (irq_counter < 0)
                      {
                         // Set Counter to Latch and Fire Interupt
                            irq_counter = irq_latch;
                            return 3;
                      }
                }
          }
    }
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

void MMC3_set_CPU_banks()
{
 // Map Program ROM
    if (prg_swap())
    {
        setCPUBanks(getNum8KRomBanks()-2,prg1,prg0,getNum8KRomBanks()-1);
    }
    else
    {
        setCPUBanks(prg0,prg1,getNum8KRomBanks()-2,getNum8KRomBanks()-1);
    }
}
void MMC3_set_PPU_banks()
{
    // Check if VROM Banks Exist and Map Them
    if(getNum1KVROMBanks() !=0 )
    {
        // Check if Swap Low and High Character ROM
        if (chr_swap())
        {
            setPPUBanks(chr4,chr5,chr6,chr7,chr01,chr01+1,chr23,chr23+1);
        }
        else
        {
            setPPUBanks(chr01,chr01+1,chr23,chr23+1,chr4,chr5,chr6,chr7);
        }
    }
    else
    {
        // No VROM Banks so Map VRAM
        if(chr_swap())
        {
            setPPUVRAMBank(0, chr4);
            setPPUVRAMBank(1, chr5);
            setPPUVRAMBank(2, chr6);
            setPPUVRAMBank(3, chr7);
            setPPUVRAMBank(4, chr01+0);
            setPPUVRAMBank(5, chr01+1);
            setPPUVRAMBank(6, chr23+0);
            setPPUVRAMBank(7, chr23+1);
        }
        else
        {
            setPPUVRAMBank(0, chr01+0);
            setPPUVRAMBank(1, chr01+1);
            setPPUVRAMBank(2, chr23+0);
            setPPUVRAMBank(3, chr23+1);
            setPPUVRAMBank(4, chr4);
            setPPUVRAMBank(5, chr5);
            setPPUVRAMBank(6, chr6);
            setPPUVRAMBank(7, chr7);
        }
    }
}
int chr_swap()
{
    return (regs[0] & 0x80)!=0;
}
int prg_swap()
{
    return (regs[0] & 0x40)!=0;
}

void saveMapperState(FILE* file)
{
    fwrite(&regs,sizeof(int),8, file);

    fputc( irq_counter & 0xFF, file);
    fputc( irq_enabled ? 0xFF : 0x00, file);
    fputc( irq_latch & 0xFF, file);
    fputc( prg0  & 0xFF, file);
    fputc( prg1  & 0xFF, file);
    fputc( chr01 & 0xFF, file);
    fputc( chr23 & 0xFF, file);
    fputc( chr4  & 0xFF, file);
    fputc( chr5  & 0xFF, file);
    fputc( chr6  & 0xFF, file);
    fputc( chr7  & 0xFF, file);
}
void loadMapperState(FILE* file)
{
    fread(&regs,sizeof(int),8, file);

    irq_counter = fgetc(file) & 0xFF;
    irq_enabled = (fgetc(file) == 0xFF)? 1: 0;
    irq_latch = fgetc(file) & 0xFF;
    prg0 = fgetc(file) & 0xFF;
    prg1 = fgetc(file) & 0xFF;
    chr01= fgetc(file) & 0xFF;
    chr23= fgetc(file) & 0xFF;
    chr4 = fgetc(file) & 0xFF;
    chr5 = fgetc(file) & 0xFF;
    chr6 = fgetc(file) & 0xFF;
    chr7 = fgetc(file) & 0xFF;
}
