#include <stdio.h>

#include "NES.h"
#include "M6502.h"

static int CYCLES[256];
static int znTable[256];

int A;
int X;
int Y;
int P;
int S;
int PC;
float cyclesPending;
float CYCLES_PER_LINE = 116.0f;

boolean halted;

#define ASL(i)\
	P &= 0x7C;\
	P |= i >> 7;\
	i <<= 1;\
	i &= 0xFF;\
	P |= znTable[i];

#define LSR(i)\
	P &= 0x7C;\
	P |= i & 0x1;\
	i >>= 1;\
	P |= znTable[i];

#define ROL(i)\
	i <<= 1;\
	i |= P & 0x1;\
	P &= 0x7C;\
	P |= i >> 8;\
	i &= 0xFF;\
	P |= znTable[i];

#define ROR(i)\
	j = P & 0x1;\
	P &= 0x7C;\
	P |= i & 0x1;\
	i >>= 1;\
	i |= j << 7;\
	P |= znTable[i];

#define increment(i)\
	i++;\
	i &= 0xff;\
	setStatusFlags(i);\

#define decrement(i)\
	i--;\
	i &= 0xff;\
	setStatusFlags(i);

#define operateAdd(i)\
	k = P & 0x1;\
	j = A + i + k;\
	P &= 0x3C;\
	P |= (~(A ^ i) & (A ^ i) & 0x80) == 0 ? 0 : 0x40;\
	P |= j <= 255 ? 0 : 0x1;\
	A = j & 0xFF;\
	P |= znTable[A];

#define operateSub(i)\
	k = ~P & 0x1;\
	j = A - i - k;\
	P &= 0x3C;\
	P |= (~(A ^ i) & (A ^ i) & 0x80) == 0 ? 0 : 0x40;\
	P |= j < 0 ? 0 : 0x1;\
	A = j & 0xFF;\
	P |= znTable[A];

#define operateCmp(i, j)\
	k = i - j;\
	P &= 0x7C;\
	P |= k < 0 ? 0 : 0x1;\
	P |= znTable[k & 0xff];

#define operateBit(i)\
	P &= 0x3D;\
	P |= i & 0xc0;\
	P |= (A & i) != 0 ? 0 : 0x2;

void emulateCPUCycles(float cycles) {
	// Declare Deficit Cycles
	cyclesPending += cycles;
	// Loop until a Horizontal Blank is encountered
	int i,j,k;
	while (cyclesPending > 0) {
		// Fetch and Execute the Next Instruction
		if (!halted) {

			// Fetch the Next Instruction Code
			int instCode = readByte(PC++);
			// Declare Variables for Handling Addresses and Values
			int address;
			int writeVal;
			// Check if an Instruction Code can be Identified
			switch (instCode) {
			case 0x00: // BRK
				address = PC + 1;
				pushWord(address);
				push(P | 0x10);
				PC = readWord(0xFFFE);
				P |= 0x04;
				P |= 0x10;
				break;
			case 0xA9: // LDA #aa

				A = byImmediate();
				setStatusFlags(A);
				break;
			case 0xA5: // LDA Zero Page

				A = readByte(byZeroPage());
				setStatusFlags(A);
				break;
			case 0xB5: // LDA $aa,X

				A = readByte(byZeroPageX());
				setStatusFlags(A);
				break;
			case 0xAD: // LDA $aaaa

				A = readByte(byAbsolute());
				setStatusFlags(A);
				break;
			case 0xBD: // LDA $aaaa,X

				A = readByte(byAbsoluteX());
				setStatusFlags(A);
				break;
			case 0xB9: // LDA $aaaa,Y

				A = readByte(byAbsoluteY());
				setStatusFlags(A);
				break;
			case 0xA1: // LDA ($aa,X)

				A = readByte(byIndirectX());
				setStatusFlags(A);
				break;
			case 0xB1: // LDA ($aa),Y

				A = readByte(byIndirectY());
				setStatusFlags(A);
				break;
			case 0xA2: // LDX #aa

				X = byImmediate();
				setStatusFlags(X);
				break;
			case 0xA6: // LDX $aa

				X = readByte(byZeroPage());
				setStatusFlags(X);
				break;
			case 0xB6: // LDX $aa,Y

				X = readByte(byZeroPageY());
				setStatusFlags(X);
				break;
			case 0xAE: // LDX $aaaa

				X = readByte(byAbsolute());
				setStatusFlags(X);
				break;
			case 0xBE: // LDX $aaaa,Y

				X = readByte(byAbsoluteY());
				setStatusFlags(X);
				break;
			case 0xA0: // LDY #aa

				Y = byImmediate();
				setStatusFlags(Y);
				break;
			case 0xA4: // LDY $aa

				Y = readByte(byZeroPage());
				setStatusFlags(Y);
				break;
			case 0xB4: // LDY $aa,X

				Y = readByte(byZeroPageX());
				setStatusFlags(Y);
				break;
			case 0xAC: // LDY $aaaa

				Y = readByte(byAbsolute());
				setStatusFlags(Y);
				break;
			case 0xBC: // LDY $aaaa,x

				Y = readByte(byAbsoluteX());
				setStatusFlags(Y);
				break;
			case 0x85: // STA $aa
				address = byZeroPage();
				writeByte(address, A);
				break;
			case 0x95: // STA $aa,X

				address = byZeroPageX();
				writeByte(address, A);
				break;
			case 0x8D: // STA $aaaa

				address = byAbsolute();
				writeByte(address, A);
				break;
			case 0x9D: // STA $aaaa,X

				address = byAbsoluteX();
				writeByte(address, A);
				break;
			case 0x99: // STA $aaaa,Y

				address = byAbsoluteY();
				writeByte(address, A);
				break;
			case 0x81: // STA ($aa,X)

				address = byIndirectX();
				writeByte(address, A);
				break;
			case 0x91: // STA ($aa),Y

				address = byIndirectY();
				writeByte(address, A);
				break;
			case 0x86: // STX $aa

				address = byZeroPage();
				writeByte(address, X);
				break;
			case 0x96: // STX $aa,Y

				address = byZeroPageY();
				writeByte(address, X);
				break;
			case 0x8E: // STX $aaaa

				address = byAbsolute();
				writeByte(address, X);
				break;
			case 0x84: // STY $aa

				address = byZeroPage();
				writeByte(address, Y);
				break;
			case 0x94: // STY $aa,X

				address = byZeroPageX();
				writeByte(address, Y);
				break;
			case 0x8C: // STY $aaaa

				address = byAbsolute();
				writeByte(address, Y);
				break;
			case 0xAA: // TAX

				X = A;
				setStatusFlags(X);
				break;
			case 0xA8: // TAY

				Y = A;
				setStatusFlags(Y);
				break;
			case 0xBA: // TSX

				X = S & 0xFF;
				setStatusFlags(X);
				break;
			case 0x8A: // TXA

				A = X;
				setStatusFlags(A);
				break;
			case 0x9A: // TXS

				S = X & 0XFF;
				break;
			case 0x98: // TYA

				A = Y;
				setStatusFlags(A);
				break;
			case 0x09: // ORA #aa

				A |= byImmediate();
				setStatusFlags(A);
				break;
			case 0x05: // ORA $aa

				address = byZeroPage();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x15: // ORA $aa,X

				address = byZeroPageX();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x0D: // ORA $aaaa

				address = byAbsolute();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x1D: // ORA $aaaa,X

				address = byAbsoluteX();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x19: // ORA $aaaa,Y

				address = byAbsoluteY();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x01: // ORA ($aa,X)

				address = byIndirectX();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x11: // ORA ($aa),Y

				address = byIndirectY();
				A |= readByte(address);
				setStatusFlags(A);
				break;
			case 0x29: // AND #aa

				A &= byImmediate();
				setStatusFlags(A);
				break;
			case 0x25: // AND $aa

				address = byZeroPage();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x35: // AND $aa,X

				address = byZeroPageX();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x2D: // AND $aaaa

				address = byAbsolute();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x3D: // AND $aaaa,X

				address = byAbsoluteX();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x39: // AND $aaaa,Y

				address = byAbsoluteY();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x21: // AND ($aa,X)

				address = byIndirectX();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x31: // AND ($aa),Y

				address = byIndirectY();
				A &= readByte(address);
				setStatusFlags(A);
				break;
			case 0x49: // EOR #aa

				A ^= byImmediate();
				setStatusFlags(A);
				break;
			case 0x45: // EOR $aa

				address = byZeroPage();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x55: // EOR $aa,X

				address = byZeroPageX();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x4D: // EOR $aaaa

				address = byAbsolute();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x5D: // EOR $aaaa,X

				address = byAbsoluteX();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x59: // EOR $aaaa,Y

				address = byAbsoluteY();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x41: // EOR ($aa,X)

				address = byIndirectX();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x51: // EOR ($aa),Y

				address = byIndirectY();
				A ^= readByte(address);
				setStatusFlags(A);
				break;
			case 0x24: // BIT $aa

				operateBit(readByte(byZeroPage()));
				break;
			case 0x2C: // BIT $aaaa

				operateBit(readByte(byAbsolute()));
				break;
			case 0x0A: // ASL A

				ASL(A);
				break;
			case 0x06: // ASL $aa

				address = byZeroPage();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ASL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x16: // ASL $aa,X

				address = byZeroPageX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ASL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x0E: // ASL $aaaa

				address = byAbsolute();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ASL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x1E: // ASL $aaaa,X

				address = byAbsoluteX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ASL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x4A: // LSR A

				LSR(A);
				break;
			case 0x46: // LSR $aa

				address = byZeroPage();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				LSR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x56: // LSR $aa,X

				address = byZeroPageX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				LSR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x4E: // LSR $aaaa

				address = byAbsolute();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				LSR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x5E: // LSR $aaaa,X

				address = byAbsoluteX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				LSR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x2A: // ROL A

				ROL(A);
				break;
			case 0x26: // ROL $aa (RWMW)

				address = byZeroPage();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x36: // ROL $aa,X (RWMW)

				address = byZeroPageX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x2E: // ROL $aaaa (RWMW)

				address = byAbsolute();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x3E: // ROL $aaaa,X (RWMW)

				address = byAbsoluteX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROL(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x6A: // ROR A

				ROR(A);
				break;
			case 0x66: // ROR $aa

				address = byZeroPage();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x76: // ROR $aa,X

				address = byZeroPageX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x6E: // ROR $aaaa

				address = byAbsolute();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROR(writeVal);
				writeByte(address, writeVal);
				break;
			case 0x7E: // ROR $aaaa,X

				address = byAbsoluteX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				ROR(writeVal);
				writeByte(address, writeVal);
				break;

			case 0x4C: // JMP $aaaa
				PC = byAbsolute();
				break;
			case 0x6C: // JMP ($aaaa)

				address = byAbsolute();

				if ((address & 0x00FF) == 0xFF)
					PC = (readByte(address & 0xFF00) << 8) | readByte(address);
				else
					PC = readWord(address);
				break;
			case 0x20: // JSR $aaaa

				address = PC + 1;
				pushWord(address);
				PC = byAbsolute();
				break;
			case 0x60: // RTS

				PC = popWord() + 1;
				break;
			case 0x40: // RTI

				P = pop();
				PC = popWord();
				break;
			case 0x48: // PHA

				push(A);
				break;
			case 0x08: // PHP

				push(P | 0x10); // SET BRK
				break;
			case 0x68: // PLA

				A = pop();
				setStatusFlags(A);
				break;
			case 0x28: // PLP

				P = pop();
				break;
			case 0x18: // CLC

				P &= 0xfe;
				break;
			case 0xD8: // CLD

				P &= 0xf7;
				break;
			case 0x58: // CLI

				P &= 0xfb;
				break;
			case 0xB8: // CLV

				P &= 0xbf;
				break;
			case 0x38: // SEC

				P |= 0x1;
				break;
			case 0xF8: // SED

				P |= 0x8;
				break;
			case 0x78: // SEI

				P |= 0x4;
				break;
			case 0xE6: // INC $aa (RWMW)

				address = byZeroPage();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				increment(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xF6: // INC $aa,X (RWMW)

				address = byZeroPageX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				increment(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xEE: // INC $aaaa (RWMW)

				address = byAbsolute();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				increment(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xFE: // INC $aaaa,X (RWMW)

				address = byAbsoluteX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				increment(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xE8: // INX

				X++;
				X &= 0xff;
				setStatusFlags(X);
				break;
			case 0xC8: // INY

				Y++;
				Y &= 0xff;
				setStatusFlags(Y);
				break;
			case 0xC6: // DEC $aa (RWMW)

				address = byZeroPage();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				decrement(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xD6: // DEC $aa,X (RWMW)

				address = byZeroPageX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				decrement(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xCE: // DEC $aaaa (RWMW)

				address = byAbsolute();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				decrement(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xDE: // DEC $aaaa,X (RWMW)

				address = byAbsoluteX();
				writeVal = readByte(address);
				writeByte(address, writeVal);
				writeVal = readByte(address);
				decrement(writeVal);
				writeByte(address, writeVal);
				break;
			case 0xCA: // DEX

				X--;
				X &= 0xff;
				setStatusFlags(X);
				break;
			case 0x88: // DEY

				Y--;
				Y &= 0xff;
				setStatusFlags(Y);
				break;
			case 0x69: // ADC #aa

				operateAdd(byImmediate());
				break;
			case 0x65: // ADC $aa

				operateAdd(readByte(byZeroPage()));
				break;
			case 0x75: // ADC $aa,X

				operateAdd(readByte(byZeroPageX()));
				break;
			case 0x6D: // ADC $aaaa

				operateAdd(readByte(byAbsolute()));
				break;
			case 0x7D: // ADC $aaaa,X

				operateAdd(readByte(byAbsoluteX()));
				break;
			case 0x79: // ADC $aaaa,Y

				operateAdd(readByte(byAbsoluteY()));
				break;
			case 0x61: // ADC ($aa,X)

				operateAdd(readByte(byIndirectX()));
				break;
			case 0x71: // ADC ($aa),Y

				operateAdd(readByte(byIndirectY()));
				break;
			case 0xEB: // SBC #aa
			case 0xE9: // SBC #aa

				operateSub(byImmediate());
				break;
			case 0xE5: // SBC $aa

				operateSub(readByte(byZeroPage()));
				break;
			case 0xF5: // SBC $aa,X

				operateSub(readByte(byZeroPageX()));
				break;
			case 0xED: // SBC $aaaa

				operateSub(readByte(byAbsolute()));
				break;
			case 0xFD: // SBC $aaaa,X

				operateSub(readByte(byAbsoluteX()));
				break;
			case 0xF9: // SBC $aaaa,Y

				operateSub(readByte(byAbsoluteY()));
				break;
			case 0xE1: // SBC ($aa,X)

				operateSub(readByte(byIndirectX()));
				break;
			case 0xF1: // SBC ($aa),Y

				operateSub(readByte(byIndirectY()));
				break;
			case 0xC9: // CMP #aa
				operateCmp(A, byImmediate());
				break;
			case 0xC5: // CMP $aa
				operateCmp(A, readByte(byZeroPage()));
				break;
			case 0xD5: // CMP $aa,X
				operateCmp(A, readByte(byZeroPageX()));
				break;
			case 0xCD: // CMP $aaaa
				operateCmp(A, readByte(byAbsolute()));
				break;
				// UP TO HERE
			case 0xDD: // CMP $aaaa,X
				operateCmp(A, readByte(byAbsoluteX()));
				break;
			case 0xD9: // CMP $aaaa,Y
				operateCmp(A, readByte(byAbsoluteY()));
				break;
			case 0xC1: // CMP ($aa,X)
				operateCmp(A, readByte(byIndirectX()));
				break;
			case 0xD1: // CMP ($aa),Y
				operateCmp(A, readByte(byIndirectY()));
				break;
			case 0xE0: // CPX #aa
				operateCmp(X, byImmediate());
				break;
			case 0xE4: // CPX $aa
				operateCmp(X, readByte(byZeroPage()));
				break;
			case 0xEC: // CPX $aaaa
				operateCmp(X, readByte(byAbsolute()));
				break;
			case 0xC0: // CPY #aa
				operateCmp(Y, byImmediate());
				break;
			case 0xC4: // CPY $aa

				operateCmp(Y, readByte(byZeroPage()));
				break;
			case 0xCC: // CPY $aaaa

				operateCmp(Y, readByte(byAbsolute()));
				break;

			case 0x90: // BCC
				branch(0x01, 0);
				break;
			case 0xB0: // BCS
				branch(0x01, 1);
				break;
			case 0xD0: // BNE
				branch(0x02, 0);
				break;
			case 0xF0: // BEQ
				branch(0x02, 1);
				break;
			case 0x10: // BPL
				branch(0x80, 0);
				break;
			case 0x30: // BMI
				branch(0x80, 1);
				break;
			case 0x50: // BVC
				branch(0x40, 0);
				break;
			case 0x70: // BVS
				branch(0x40, 1);
				break;

			case 0x1A: // UNDOCUMENTED : NOP
			case 0x3A:
			case 0x5A:
			case 0x7A:
			case 0xDA:
			case 0xEA:
			case 0xFA:

				break;
			case 0x02: // UNDOCUMENTED : HLT
			case 0x12:
			case 0x22:
			case 0x32:
			case 0x42:
			case 0x52:
			case 0x62:
			case 0x72:
			case 0x92:
			case 0xB2:
			case 0xD2:
			case 0xF2:

				halted = true
				;
				PC--;
				break;
			default: // Unknown OpCode so Hang
				PC--;
				break;
			}
			// Decrement Cycles by number of Cycles in Instruction
			cyclesPending -= CYCLES[instCode];
		} else
			cyclesPending--;
		// Check for a Stop Request
	}
}

void eatCycles(int cycles) {
	cyclesPending -= cycles;
}
/**
 *
 * <P>
 * Perform a Non Maskable Interrupt.
 * </P>
 *
 */
void NMI() {
	pushWord(PC);
	push(P & 0xEF); // CLEAR BRK
	PC = readWord(0xFFFA);
	cyclesPending += 7;
}

/**
 *
 * <P>
 * Perform a IRQ/BRK Interrupt.
 * </P>
 *
 */
void IRQ() {
	if ((P & 0x4) == 0x00) {
		pushWord(PC);
		push(P & 0xEF); // CLEAR BRK
		PC = readWord(0xFFFE);
		P |= 0x04;
		cyclesPending += 7;
	}
}

/**
 *
 * <P>
 * Correct the CPU Cycles for a Couple of Odd Games.
 * </P>
 *
 */
void correctCPUCycles() {
}

/**
 *
 * <P>
 * Reset the internal CPU registers.
 * </P>
 *
 */
void initResetCPU() {
	// Correct CPU Cycles for Odd Games
	correctCPUCycles();
	// Reset the CPU Registers
	A = 0x00;
	X = 0x00;
	Y = 0x00;
	P = 0x04;
	S = 0xFF;
	halted = false;
	// Read the Reset Vector for PC Address
	PC = readWord(0xFFFC);
	int r;
	r = readByte(0xFFFC);
	r = readByte(0xFFFD);
}

// //////////////////////////////////////////////////////////////////////////////
//
// Addressing Mode Functions
//
// //////////////////////////////////////////////////////////////////////////////
/**
 *
 * <P>
 * Get value by Immediate Mode Addressing - #$00
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byImmediate() {
	int i = readByte(PC++);

	return i;
}

/**
 *
 * <P>
 * Get value by Absolute Mode Addressing - $aaaa
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byAbsolute() {
	int address = readWord(PC);

	PC += 2;
	return address;
}

/**
 *
 * <P>
 * Get value by Absolute Y Mode Addressing - $aaaa,Y
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byAbsoluteY() {
	int i = byAbsolute();

	int j = i + Y;
	checkPageBoundaryCrossing(i, j);
	return j;
}

/**
 *
 * <P>
 * Get value by Absolute X Mode Addressing - $aaaa,X
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byAbsoluteX() {
	int i = byAbsolute();

	int j = i + X;
	checkPageBoundaryCrossing(i, j);
	return j;
}

/**
 *
 * <P>
 * Get value by Zero Page Mode Addressing - $aa
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byZeroPage() {
	int address = readByte(PC++);

	return address;
}

/**
 *
 * <P>
 * Get value by Zero Page X Mode Addressing - $aa,X
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byZeroPageX() {
	int address = readByte(PC++);

	return (address + X) & 0xff;
}

/**
 *
 * <P>
 * Get value by Zero Page Y Mode Addressing - $aa,Y
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byZeroPageY() {
	int address = readByte(PC++);

	return (address + Y) & 0xff;
}

/**
 *
 * <P>
 * Get value by Indirect X Mode Addressing - ($aa,X)
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byIndirectX() {
	int address = readByte(PC++);

	address += X;
	address &= 0xFF;
	return readWord(address);
}

/**
 *
 * <P>
 * Get value by Indirect Y Mode Addressing - ($aa),Y
 * </P>
 *
 * @return The value by the specified addressing mode in relation to the
 *         current PC.
 *
 */
int byIndirectY() {
	int address = readByte(PC++);

	address = readWord(address);
	checkPageBoundaryCrossing(address, address + Y);
	return address + Y;
}

// //////////////////////////////////////////////////////////////////////////////
//
// Utility Functions
//
// //////////////////////////////////////////////////////////////////////////////
/**
 *
 * <P>
 * Decrement the number of cycles pending if over a page boundary.
 * </P>
 *
 * @param address1
 *            The first address.
 * @param address2
 *            The second address.
 *
 */
void checkPageBoundaryCrossing(int address1, int address2) {
	if (((address2 ^ address1) & 0x100) != 0)
		cyclesPending--;
}

/**
 *
 * <P>
 * Set the Zero and Negative Status Flags.
 * </P>
 *
 * @param value
 *            The value used to determine the Status Flags.
 *
 */
void setStatusFlags(int value) {
	P &= 0x7D;
	P |= znTable[value];
}

/**
 *
 * <P>
 * Function for Handling Branches
 * </P>
 *
 * @param flagNum
 *            The byte value to compare.
 * @param flagVal
 *            The expected truth value for a branch.
 *
 */
void branch(int flagNum, boolean flagVal) {
	int offset = (char) readByte(PC++);

	if (((P & flagNum) != 0) == flagVal) {
		checkPageBoundaryCrossing(PC + offset, PC);
		PC = PC + offset;
		cyclesPending--;
	}
}

/**
 *
 * <P>
 * Push a value onto the Stack.
 * </P>
 *
 * @param stackVal
 *            The value to push.
 *
 */
void push(int stackVal) {
	writeByte(S + 256, stackVal);
	S--;
	S &= 0xff;
}

/**
 *
 * <P>
 * Pop a value from the Stack.
 * </P>
 *
 * @return The value on top of the Stack.
 *
 */
int pop() {
	S++;
	S &= 0xff;
	return readByte(S + 256);
}

/**
 *
 * <P>
 * Push a Word onto the Stack.
 * </P>
 *
 * @param stackVal
 *            The 16 bit word to push.
 *
 */
void pushWord(int stackVal) {
	push((stackVal >> 8) & 0xFF);
	push(stackVal & 0xFF);
}

/**
 *
 * <P>
 * Pop a Word from the Stack.
 * </P>
 *
 * @return The 16 bit word on top of the Stack.
 *
 */
int popWord() {
	return pop() + pop() * 256;
}

#ifndef Z_FLAG

#define	C_FLAG	  0x01         /* 1: Carry occured           */
#define	Z_FLAG	  0x02         /* 1: Result is zero          */
#define	I_FLAG	  0x04         /* 1: Interrupts disabled     */
#define	D_FLAG	  0x08         /* 1: Decimal mode            */
#define	B_FLAG	  0x10         /* Break [0 on stk after int] */
#define	R_FLAG	  0x20         /* Always 1                   */
#define	V_FLAG	  0x40         /* 1: Overflow occured        */
#define	N_FLAG	  0x80         /* 1: Result is negative      */

#endif
static int CYCLES[256] = { 7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6, 2,
		5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7, 6, 6, 2, 8, 3, 3, 5, 5, 4,
		2, 2, 2, 4, 4, 6, 6, 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7, 6,
		6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6, 2, 5, 2, 8, 4, 4, 6, 6, 2,
		4, 2, 7, 5, 5, 7, 7, 6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6, 2,
		5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7, 2, 6, 2, 6, 3, 3, 3, 3, 2,
		2, 2, 2, 4, 4, 4, 4, 2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5, 2,
		6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4, 2, 5, 2, 5, 4, 4, 4, 4, 2,
		4, 2, 5, 4, 4, 4, 4, 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, 2,
		5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7, 2, 6, 2, 8, 3, 3, 5, 5, 2,
		2, 2, 2, 4, 4, 6, 6, 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7 };

static int
		znTable[256] = { Z_FLAG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG, N_FLAG,
				N_FLAG, N_FLAG, N_FLAG, };
