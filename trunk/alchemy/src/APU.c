#include "NES.h"
#include "APU.h"
#include <math.h>

#define APUQUEUE_SIZE 4096
/**
 * The APU Square 1 Register Addresses.
 */
#define APU_WRA0 0x4000
#define APU_WRA1 0x4001
#define APU_WRA2 0x4002
#define APU_WRA3 0x4003
/**
 * The APU Square 2 Register Addresses.
 */
#define APU_WRB0 0x4004
#define APU_WRB1 0x4005
#define APU_WRB2 0x4006
#define APU_WRB3 0x4007
/**
 * The APU Triangle Register Addresses.
 */
#define APU_WRC0 0x4008
#define APU_WRC2 0x400A
#define APU_WRC3 0x400B
/**
 * The APU Noise Register Addresses.
 */
#define APU_WRD0 0x400C
#define APU_WRD2 0x400E
#define APU_WRD3 0x400F
/**
 * The DMC Register Addresses
 */
#define APU_WRE0 0x4010
#define APU_WRE1 0x4011
#define APU_WRE2 0x4012
#define APU_WRE3 0x4013
/**
 * The APU Enable Disable Register.
 */
#define APU_EREG 0x4015
/**
 * The frequency of the APU.
 */
#define APU_BASEFREQ 1789772.5
// ////////////////////////////////////////////////////////////////////
//
// References to NESCafe
//
// ////////////////////////////////////////////////////////////////////

boolean enableAllSound = true;
boolean enableSquare1 = true;
boolean enableSquare2 = true;
boolean enableTriangle = true;
boolean enableNoise = true;
boolean enableDMC = true;
boolean soundEngineLoaded = false;

// ////////////////////////////////////////////////////////////////////
//
// The Sound Channels
//
// ////////////////////////////////////////////////////////////////////
/**
 * Square (Rectangle) Channel 1 and 2
 */
Rectangle rectangle[2];
Triangle triangle;
Noise noise;
DMC dmc;
// ////////////////////////////////////////////////////////////////////
//
// Sound Queueing Structures
//
// ////////////////////////////////////////////////////////////////////
/**
 * The queue of Sound Messages
 */
Message queue[APUQUEUE_SIZE];

int qHead = 0;
int qTail = 0;
// ////////////////////////////////////////////////////////////////////
//
// APU Status Information
//
// ////////////////////////////////////////////////////////////////////
int cycleRate = 0;
int enableReg = 0;
int numOfSamples = 0;
int refreshRate;
int sampleRate = 0;
int nextSample = 0x00;
int prevSample = 0x00;
int apuCntRate = 5;
// ////////////////////////////////////////////////////////////////////
//
// Lookup Tables
//
// ////////////////////////////////////////////////////////////////////
int decayLUT[16];
int dutyCycleLUT[4] = { 2, 4, 8, 12 };
int freqLimit[] = { 0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1,
		0x7E0, 0x7F0 };
int noiseFreqLUT[] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254,
		380, 508, 762, 1016, 2034, 4068 };
int noiseLongLUT[0x7FFF];
int noiseShortLUT[93];
int sampleLengthLUT[32];
int triLengthLUT[128];
int vblankLength[] = { 5, 127, 10, 1, 19, 2, 40, 3, 80, 4, 30, 5,
		7, 6, 13, 7, 6, 8, 12, 9, 24, 10, 48, 11, 96, 12, 36, 13, 8, 14,
		16, 15 };
int dmc_clocks[] = { 428, 380, 340, 320, 286, 254, 226, 214, 190,
		160, 142, 128, 106, 85, 72, 54 };
////////////////////////////////
//
// functions
//
////////////////////////////////

void buildLUTs(int numSamples) {
	int i;
	// The LUT for Enveloping and Frequency Sweeps
	for (i = 0; i < 16; i++)
		decayLUT[i] = numSamples * (i + 1) * 5;
	// The LUT used for the Number of Samples for Note Length
	for (i = 0; i < 32; i++)
		sampleLengthLUT[i] = vblankLength[i] * numSamples * 5;
	// Linear Length Counter LUT for Triangle (Length in Frames =
	// 0.25*LoadValue)
	for (i = 0; i < 128; i++)
		triLengthLUT[i] = (int) (0.25 * (i * numSamples)) * 5;
	// Produce the Random Number Sets for Long and Short White Noise Samples
	genRandomSamples(0x7FFF);
	genRandomSamples(93);
}

/**
 * Slowly Decrement (Decay) a Volume
 */
int decayVolume(int vol) {
	return ((vol) - ((vol) >> 7));
}

/**
 * <P>
 * Denqueue a Sound Message from the Buffer.
 * </P>
 */
Message dequeue() {
	// Mark Index to Return
	int index = qTail;
	// Increment the Tail of the Cyclic Queue
	qTail = (qTail + 1) & (APUQUEUE_SIZE - 1);
	// Return the Marked Message
	return queue[index];
}

/**
 * <P>
 * Enqueue a Sound Message to the Buffer.
 * </P>
 */
void enqueue(Message message) {
	// Set Message at Head of Queue
	queue[qHead] = message;
	// Increment Head of Queue Pointer
	qHead = (qHead + 1) & (APUQUEUE_SIZE - 1);
}

/**
 * <P>
 * Generate "Random" Samples for White Noise Channel.
 * </P>
 * This information was available from Message 1083 on eGroups.com for
 * NesDev
 */
void genRandomSamples(int count) {
	// Declare Shift Register Seed Value
	int sreg = 0x4000;
	// Declare Variables for Bits 0, 1 and 14
	int bit0, bit1, bit6, bit14;
	// Declare Variable for index into Random Value Buffer
	int bufIndex = 0;
	// Generate Short or Long Samples
	if (count == 93) {
		// Generate Short Samples
		while (count > 0) {
			count--;
			bit0 = sreg & 1;
			bit6 = (sreg & 0x40) >> 6;
			bit14 = (bit0 ^ bit6);
			sreg >>= 1;
			sreg |= (bit14 << 14);
			noiseShortLUT[bufIndex++] = bit0 ^ 1;
		}
	} else {
		// Generate Long Samples
		while (count > 0) {
			count--;
			bit0 = sreg & 1;
			bit1 = (sreg & 2) >> 1;
			bit14 = (bit0 ^ bit1);
			sreg >>= 1;
			sreg |= (bit14 << 14);
			noiseLongLUT[bufIndex++] = bit0 ^ 1;
		}
	}
}

/**
 * Initialise the Sound Engine.
 */
void initAPU(int SampleRate, int RefreshRate) {
	// Copy Settings
	sampleRate = SampleRate;
	refreshRate = RefreshRate;
	// Calculate the Number of Samples to Produce at Each Refresh
	numOfSamples = sampleRate / refreshRate;
	// Calculate the Cycle Rate
	cycleRate = floor(APU_BASEFREQ * 65536.0 / sampleRate);
	// Build the Look Up Tables
	buildLUTs(numOfSamples);
}
/**
 * Process a White Noise Sound Channel and return Volume.
 */
int processNoise(Noise chan) {
	// Declare Variable for Noise
	int noiseBit = 0;
	int output = 0;
	// Decay the Existing Channel Volume
	chan.outputVol = decayVolume(chan.outputVol);
	// If the Channel is not Enabled then return the Current Volume
	if (!chan.enabled || (chan.vblLength == 0))
		return ((chan.outputVol + chan.outputVol + chan.outputVol) >> 2);
	// Only Decrement Vertical Blank Linear Counter if Note is not Held
	if (!chan.holdNote)
		chan.vblLength -= apuCntRate;
	// In Last Refresh Envelope Counter should have Updated 4 Times (Clocked
	// at 240Hz)
	chan.envPhase -= 4 * apuCntRate;
	// Eat up those Cycles
	while (chan.envPhase < 0) {
		// Increment Cycles by Decay Frequency
		chan.envPhase += (chan.envDecay + 1);
		// If Note is Being Held Loop Back to 0
		if (chan.holdNote) {
			chan.envVol = (chan.envVol + 1) & 0x0F;
		} else if (chan.envVol < 0x0F) {
			chan.envVol++;
		}
	}
	// State the Number of Cycles needed in this Sample
	chan.phaseAcc -= cycleRate;
	// If none are needed then Return
	if (chan.phaseAcc >= 0)
		return ((chan.outputVol + chan.outputVol + chan.outputVol) >> 2);
	// Eat up the Cycles
	while (chan.phaseAcc < 0) {
		// Increment Cycles Needed by Frequency of Channel
		chan.phaseAcc += (chan.freq + 1);
		// Increment the Position within the Random Sampled Data
		chan.curPos++;
		// Check Limits of Array
		if (chan.shortSample) {
			if (chan.curPos == 93)
				chan.curPos = 0;
		} else {
			if (chan.curPos == 0x7FFF)
				chan.curPos = 0;
		}
	}
	// Sets Output Depending on Whether from a Fixed Volume or the Decaying
	// Envelope
	if (chan.fixedEnvelope)
		output = chan.volume << 8;
	else
		output = (chan.envVol ^ 0x0F) << 8;
	// Get the Current Noise Bit from the Sampled Data
	if (chan.shortSample)
		noiseBit = noiseShortLUT[chan.curPos];
	else
		noiseBit = noiseLongLUT[chan.curPos];
	// Allow that Noise Bit to Determine Positive or Negative Cycle
	if (noiseBit != 0) {
		chan.outputVol = output;
	} else {
		chan.outputVol = -output;
	}
	// Return the Output Volume
	return ((chan.outputVol + chan.outputVol + chan.outputVol) >> 2);
}

/**
 * Process a Rectangle Sound Channel and return Volume.
 */
int processRectangle(Rectangle chan) {
	// Declare Variable for Output Volume
	int output = 0;
	// Decay the Existing Channel Volume
	chan.outputVol = decayVolume(chan.outputVol);
	// If the Channel is not Enabled then return the Current Volume
	if (!chan.enabled || (chan.vblLength == 0))
		return chan.outputVol;
	// Only Decrement Vertical Blank Linear Counter if Note is not Held
	if (!chan.holdNote)
		chan.vblLength -= apuCntRate;
	// In Last Refresh Envelope Counter should have Updated 4 Times (Clocked
	// at 240Hz)
	chan.envPhase -= 4 * apuCntRate;
	// Eat up those Cycles
	while (chan.envPhase < 0) {
		// Increment Cycles by Decay Frequency
		chan.envPhase += (chan.envDecay + 1);
		// If Note is Being Held Loop Back to 0
		if (chan.holdNote) {
			chan.envVol = (chan.envVol + 1) & 0x0F;
		} else if (chan.envVol < 0x0F) {
			chan.envVol++;
		}
	}
	// Determine if the Channel is Audible
	if (chan.freq < 8 || (!chan.sweepInc && chan.freq > chan.freqLimit))
		return chan.outputVol;
	// Sweep the Wavelength if Sweep Unit is Active
	if (chan.sweepOn && (chan.sweepShifts != 0)) {
		// Refresh Sweep is Clocked at 120Hz (Should be Twice in Last
		// Refresh)
		chan.sweepPhase -= 2 * apuCntRate;
		// Ensure Sweep Decay Frequency is Positive
		if (chan.sweepDecay <= 0)
			chan.sweepDecay = 1;
		// Eat up the Cycles
		while (chan.sweepPhase < 0) {
			// Increment by Sweep Decay Frequency
			chan.sweepPhase += chan.sweepDecay;
			// Check if we Increase or Decrease the Wavelength
			if (chan.sweepInc) {
				// Ramp Up
				chan.freq -= (chan.freq >> chan.sweepShifts);
			} else {
				// Ramp down
				chan.freq += (chan.freq >> chan.sweepShifts);
			}
		}
	}
	// State the Number of Cycles needed in this Sample
	chan.phaseacc -= cycleRate;
	// If none are needed then Return
	if (chan.phaseacc >= 0)
		return chan.outputVol;
	// Eat up the Cycles
	while (chan.phaseacc < 0) {
		// Increment Cycles Needed by Frequency of Channel
		chan.phaseacc += (chan.freq + 1) << 16;
		// Increments the Channel Adder that Keeps Track of Pos/Neg
		// Amplitude
		chan.adder = (chan.adder + 1) & 0x0F;
	}
	// Sets Output Depending on Whether from a Fixed Volume or the Decaying
	// Envelope
	if (chan.fixedEnvelope)
		output = chan.volume << 8;
	else
		output = (chan.envVol ^ 0x0F) << 8;
	// Determine the Positive or Negative Cycle and Sets Output Volume
	// Accordingly
	if (chan.adder == 0)
		chan.outputVol = output;
	// The Adder has Reached the Value at which to Flip
	else if (chan.adder == chan.dutyFlip)
		chan.outputVol = -output;
	// Return the Output Volume
	return chan.outputVol;
}

/**
 * Process the Delta Modulation Channel and Return Volume
 */
int processDMC(DMC chan) {
	// Declare Delta bit Variables
	int delta_bit;
	double total;
	double sample_weight;
	// Decay the Existing Channel Volume
	chan.outputVol = decayVolume(chan.outputVol);
	// Only Process when Channel is Alive
	if (chan.dmaLength != 0) {
		// Adjust Sample
		sample_weight = chan.phaseacc;
		if (sample_weight > cycleRate) {
			sample_weight = cycleRate;
		}
		total = (chan.regs[1] << 8) * sample_weight;
		// State the Number of Cycles needed in this Sample
		chan.phaseacc -= cycleRate;
		// Eat up the Cycles
		while (chan.phaseacc < 0) {
			// Increment Cycles Needed by Frequency of Channel
			chan.phaseacc += (chan.freq);
			if (0 == (chan.dmaLength & 7)) {
				// Read Sample
				chan.curByte = readByte(chan.address);
				// Steal a Cycle for Each Read
				eatCycles(1);
				// Index Next Byte
				if (0xFFFF == chan.address) {
					chan.address = 0x8000;
				} else {
					chan.address++;
				}
			}
			// Check if at End of Sample
			if (--chan.dmaLength == 0) {
				if (chan.looping) {
					// Reload Data
					chan.address = chan.cachedAddr;
					chan.dmaLength = chan.cachedDMALength;
					chan.irqOccurred = false;
				} else {
					// Check if we Should Generate IRQ
					if (chan.irqGen) {
						chan.irqOccurred = true;
					}
					// Adjust Sample
					sample_weight = chan.freq - chan.phaseacc;
					total += (chan.regs[1] << 8) * sample_weight;
					while (chan.phaseacc < 0)
						chan.phaseacc += chan.freq;
					// Enable Channel
					chan.enabled = false;
					break;
				}
			}
			// Delta Bit Processing
			delta_bit = (chan.dmaLength & 7) ^ 7;
			// Positive Delta
			if ((chan.curByte & (1 << delta_bit)) != 0) {
				if (chan.regs[1] < 0x7D) {
					chan.regs[1] += 2;
					chan.outputVol += (2 << 8);
				}
			}
			// Negative Delta
			else {
				if (chan.regs[1] > 1) {
					chan.regs[1] -= 2;
					chan.outputVol -= (2 << 8);
				}
			}
			// Adjust Sample
			sample_weight = chan.freq;
			if (chan.phaseacc > 0) {
				sample_weight -= chan.phaseacc;
			}
			total += (chan.regs[1] << 8) * sample_weight;
		}
		chan.outputVol = floor(total / cycleRate + 0.5);
	} else {
		chan.outputVol = chan.regs[1] << 8;
	}
	// Return Volume
	return ((chan.outputVol + chan.outputVol + chan.outputVol) >> 2);
}

/**
 * Process the a Triangle Sound Channel and return Volume.
 */
int processTriangle(Triangle chan) {
	// Declare Variable for Output Volume
	int output = 0;
	// Decay the Existing Channel Volume
	chan.outputVol = decayVolume(chan.outputVol);
	// If the Channel is not Enabled then return the Current Volume
	if (!chan.enabled || (chan.vblLength == 0))
		return chan.outputVol + (chan.outputVol >> 2);
	// Check if the Linear Counter has Started
	if (chan.counterStarted) {
		// Pull-down the Linear Counter
		if (chan.linearLength > 0)
			chan.linearLength -= 4 * apuCntRate;
		// Pull-down the Vertical Blank Counter (Length Counter)
		if ((chan.vblLength > 0) && !chan.holdNote)
			chan.vblLength -= apuCntRate;
	}
	// Decrement the Write Latency to Start the Linear Counter
	else if (!chan.holdNote && (chan.writeLatency != 0)) {
		chan.writeLatency--;
		if (chan.writeLatency == 0)
			chan.counterStarted = true;
	}
	// Check if Channel is Audible
	if ((chan.linearLength == 0) || chan.freq < 0x40000) {
		return chan.outputVol + (chan.outputVol >> 2);
	}
	// State the Number of Cycles needed in this Sample
	chan.phaseacc -= cycleRate;
	// Eat up the Cycles
	while (chan.phaseacc < 0) {
		// Increment Cycles Needed by Frequency of Channel
		chan.phaseacc += chan.freq + 1;
		// Increments the Channel Adder that Keeps Track of Pos/Neg
		// Amplitude
		chan.adder = (chan.adder + 1) & 0x1F;
		// Negative Cycle if Adder is Between 0x10 and 0x1F
		if ((chan.adder & 0x10) != 0)
			chan.outputVol -= 0x200;
		// Positive Cycle if Adder is Between 0x00 and 0x0F
		else
			chan.outputVol += 0x200;
	}
	// Return the Output Volume (Amplitude)
	return chan.outputVol + (chan.outputVol >> 2);
}

/**
 * <P>
 * Read from a specified address mapped to the APU.
 * </P>
 */
int apuRead(int address) {
	// Declare variable for Return Value
	int value = 0;
	// Determine the Address to Read from
	switch (address) {
	case APU_EREG: // The APU Enable Register
		// Check if Rectangle Channel 1 is Playing
		if (rectangle[0].enabled && (rectangle[0].vblLength > 0))
			value |= 0x01;
		// Check if Rectangle Channel 2 is Playing
		if (rectangle[1].enabled && (rectangle[1].vblLength > 0))
			value |= 0x02;
		// Check if Triangle Channel is Playing
		if (triangle.enabled && (triangle.vblLength > 0))
			value |= 0x04;
		// Check if Noise Channel is Playing
		if (noise.enabled && (noise.vblLength > 0))
			value |= 0x08;
		// Check if the DMC is Playing
		if (dmc.enabled)
			value |= 0x10;
		// Check for DMC IRQ
		if (dmc.irqOccurred)
			value |= 0x80;
		// Return the Value
		return value;
	default: // No other Channels are Readable
		// Heavy Capacitance on Data Bus (Return MSB)
		return (address >> 8);
	}
}

/**
 * <P>
 * Refresh the Sound Hardware.
 * </P>
 */
void playAPUSound(short* buf, int samples){
	// Create a Sound Message Object
	Message mess;
	int i;
	for(i=0;i<samples;i++){
		// Check for Any New Messages
		while (qHead != qTail) {
			// If Any Exist then Dequeue and Resolve them
			mess = dequeue();
			regWrite(mess.address, mess.value);
		}
		// Process Each Sound Channel
		int accum = 0;
		if (enableSquare1)
			accum += processRectangle(rectangle[0]);
		if (enableSquare2)
			accum += processRectangle(rectangle[1]);
		if (enableTriangle)
			accum += processTriangle(triangle);
		if (enableNoise)
			accum += processNoise(noise);
		if (enableDMC)
			accum += processDMC(dmc);
		// Implement a LowPass Filter and Smooth the Notes
		nextSample = accum;
		accum += prevSample;
		accum >>= 1;
		prevSample = nextSample;
		// Small Volume Increase
		accum <<= 1;
		// Check for Amplitude too High or Low
		if (accum > 0x7FFF)
			accum = 0x7FFF;
		else if (accum < -0x8000)
			accum = -0x8000;
		// Write the Value to the Sound Engine
		buf[i] = (short)accum;
	}
}

/**
 * <P>
 * Write to a APU Sound Register.
 * </P>
 */
void regWrite(int address, int value) {
	// Declare Variable for Determining Rectangle Channel Number
	int chan = 0;
	// Determine Address to be Written to
	switch (address) {
	// ////////////////////////////////////////////////////////////////////
	//
	// Rectangle Control
	//
	// ////////////////////////////////////////////////////////////////////
	case APU_WRA0: // Rectangle Control Register 1
	case APU_WRB0:
		// Determine the Channel Number and Record Entry
		chan = ((address & 4) != 0) ? 1 : 0;
		rectangle[chan].regs[0] = value;
		// Set the Volume of the Channel
		rectangle[chan].volume = value & 0x0F;
		// Set the Envelope Decay Frequency based on the Lookup Table
		rectangle[chan].envDecay = decayLUT[value & 0x0F];
		// Set Envelope Decay Looping Enable
		rectangle[chan].holdNote = ((value & 0x20) != 0);
		// Set whether the Envelope will Decay or is Fixed
		rectangle[chan].fixedEnvelope = ((value & 0x10) != 0);
		// Sets the Duty Cycle Type from the Lookup Table
		rectangle[chan].dutyFlip = dutyCycleLUT[value >> 6];
		// Return
		return;
	case APU_WRA1: // Rectangle Sweep Control Register
	case APU_WRB1:
		// Determine the Channel Number and Record Entry
		chan = ((address & 4) != 0) ? 1 : 0;
		rectangle[chan].regs[1] = value;
		// Bit 7 Sets whether Sweeping is Active
		rectangle[chan].sweepOn = (value & 0x80) != 0;
		// Determines the Right Shift amount of the New Frequency
		rectangle[chan].sweepShifts = value & 7;
		// Determines the Sweep Refresh Rate
		rectangle[chan].sweepDecay = decayLUT[(value >> 4) & 7];
		// Determines whether Sweeps will Increase or Decrease Wavelength
		rectangle[chan].sweepInc = (value & 0x08) != 0;
		// The Frequency Limit for the Corresponding Shift
		rectangle[chan].freqLimit = freqLimit[value & 7];
		// Return
		return;
	case APU_WRA2: // Rectangle Fine Tune Register
	case APU_WRB2:
		// Determine the Channel Number and Record Entry
		chan = ((address & 4) != 0) ? 1 : 0;
		rectangle[chan].regs[2] = value;
		// Set Bits 0-7 of the Channel Frequency
		rectangle[chan].freq = (rectangle[chan].freq & 0xFF00) | value;
		return;
	case APU_WRA3: // Rectangle Course Tune Register
	case APU_WRB3:
		// Determine the Channel Number and Record Entry
		chan = ((address & 4) != 0) ? 1 : 0;
		rectangle[chan].regs[3] = value;
		// Set the Length Counter as the number of Samples until Silence
		rectangle[chan].vblLength = sampleLengthLUT[value >> 3];
		// Clear the Envelope Volume
		rectangle[chan].envVol = 0;
		// Set Bits 8-A of the Channel Frequency
		rectangle[chan].freq = ((value & 7) << 8)
				| (rectangle[chan].freq & 0xFF);
		// Reset Counter for Determining whether in a Positive or Negative
		// Cycle
		rectangle[chan].adder = 0;
		// Return
		return;
		// ////////////////////////////////////////////////////////////////////
		//
		// Triangle Control
		//
		// ////////////////////////////////////////////////////////////////////
	case APU_WRC0: // Triangle Control Register One
		// Record the Register Value
		triangle.regs[0] = value;
		// Hold the Note if the Counter has Started
		triangle.holdNote = (value & 0x80) != 0;
		// Sets the Length of the Note if in Load Mode (Counter hasn't
		// Started)
		if (!triangle.counterStarted && (triangle.vblLength > 0))
			triangle.linearLength = triLengthLUT[value & 0x7F];
		// Return
		return;

	case APU_WRC2:
		triangle.regs[1] = value;
		// Set the 8 LSB of wavelength for Triangle Channel
		triangle.freq = ((((triangle.regs[2] & 7) << 8) + value) + 1) << 16;
		// Return
		return;
	case APU_WRC3: // Triangle Frequency Register Two
		// Record the Register Value
		triangle.regs[2] = value;
		// Calculate the Latent Period Before the Counter is Started
		triangle.writeLatency = (int) (228 / ((cycleRate) >> 16));
		// The 3 MS Bits of the Wavelength for Triangle Channel
		triangle.freq = ((((value & 7) << 8) + triangle.regs[1]) + 1) << 16;
		// Set the Length Counter as the number of Samples until Silence
		triangle.vblLength = sampleLengthLUT[value >> 3];
		// Stop and Reset the Linear Counter
		triangle.counterStarted = false;
		triangle.linearLength = triLengthLUT[triangle.regs[0] & 0x7F];
		return;
		// ////////////////////////////////////////////////////////////////////
		//
		// White Noise Control
		//
		// ////////////////////////////////////////////////////////////////////
	case APU_WRD0: // White Noise Control 1
		// Record the Register Value
		noise.regs[0] = value;
		// Set the Envelope Decay Rate
		noise.envDecay = decayLUT[value & 0x0F];
		// Set the Envelope Counter Clock Disable
		noise.holdNote = (value & 0x20) != 0;
		// Set the Envelope Decay Disable
		noise.fixedEnvelope = (value & 0x10) != 0;
		// Set the Volume
		noise.volume = value & 0x0F;
		return;
	case APU_WRD2: // White Noise Control 2
		// Record the Register Value
		noise.regs[1] = value;
		// Set the Sample Frequency
		noise.freq = ((noiseFreqLUT[value & 0x0F]) << 16);
		// Set Random Number Type Generation
		if (((value & 0x80) != 0) && !noise.shortSample) {
			genRandomSamples(93);
			noise.curPos = 0;
		}
		noise.shortSample = (value & 0x80) != 0;
		// Return
		return;
	case APU_WRD3:
		// Record the Register Value
		noise.regs[2] = value;
		// Set the Length Counter as the number of Samples until Silence
		noise.vblLength = sampleLengthLUT[value >> 3];
		noise.envVol = 0;
		break;
	// ////////////////////////////////////////////////////////////////////
	//
	// Delta Modulation Channel
	//
	// ////////////////////////////////////////////////////////////////////
	case APU_WRE0:
		// Record the Register Value
		dmc.regs[0] = value;
		// Set the Sample Frequency
		dmc.freq = ((dmc_clocks[value & 0x0F]) << 16);
		// Set the Looping
		dmc.looping = ((value & 0x40) != 0);
		// Handle Interrupts
		if ((value & 0x80) != 0) {
			dmc.irqGen = true;
		} else {
			dmc.irqGen = false;
			dmc.irqOccurred = false;
		}
		break;
	case APU_WRE1: // 7-bit DAC
		// Add the Delta Between Current Output and Value
		value &= 0x7F;
		dmc.outputVol += ((value - dmc.regs[1]) << 8);
		dmc.regs[1] = value;
		break;
	case APU_WRE2:
		dmc.regs[2] = value;
		dmc.cachedAddr = 0xC000 + (value << 6);
		break;
	case APU_WRE3:
		dmc.regs[3] = value;
		dmc.cachedDMALength = ((value << 4) + 1) << 3;
		break;
	// ////////////////////////////////////////////////////////////////////
	//
	// Enable Control
	//
	// ////////////////////////////////////////////////////////////////////
	case APU_EREG: // Enable Disable Register
		// Enable or Disable the DMC
		dmc.enabled = ((value & 0x10) != 0);
		if (dmc.enabled) {
			if (0 == dmc.dmaLength) {
				// Reload DMC
				dmc.address = dmc.cachedAddr;
				dmc.dmaLength = dmc.cachedDMALength;
				dmc.irqOccurred = false;
			}
		} else {
			dmc.dmaLength = 0;
			dmc.irqOccurred = false;
		}
		// Record the Value for when Reading
		enableReg = value;
		// Enable or Disable the Rectangle Channels
		for (chan = 0; chan < 2; chan++) {
			if ((value & (1 << chan)) != 0) {
				rectangle[chan].enabled = true;
			} else {
				rectangle[chan].enabled = false;
				rectangle[chan].vblLength = 0;
			}
		}
		// Enable or Disable the Triangle Channel
		if ((value & 0x04) != 0) {
			triangle.enabled = true;
		} else {
			triangle.enabled = false;
			triangle.vblLength = 0;
			triangle.linearLength = 0;
			triangle.counterStarted = false;
			triangle.writeLatency = 0;
		}
		// Enable or Disable the White Noise Channel
		if ((value & 0x08) != 0) {
			noise.enabled = true;
		} else {
			noise.enabled = false;
			noise.vblLength = 0;
		}
		return;
		// ////////////////////////////////////////////////////////////////////
		//
		// 0x4017
		//
		// ////////////////////////////////////////////////////////////////////
	case 0x4017:
		if ((value & 0x80) != 0)
			apuCntRate = 4;
		else
			apuCntRate = 5;
		return;
	}
}

/**
 * <P>
 * Reset the Sound Hardware.
 * </P>
 */
void resetAPU() {
	// Don't Log Messages if no Sound Engine
	if (!soundEngineLoaded || !enableAllSound)
		return;
	// Reset the Enable Register
	enableReg = 0x00;
	// Reset the Rectangle Channels
	int i;
	for (i = 0; i < 2; i++) {
		rectangle[i].enabled = false;
		rectangle[i].vblLength = 0;
		rectangle[i].adder = 0;
		rectangle[i].dutyFlip = 0;
		rectangle[i].envDecay = 0;
		rectangle[i].envPhase = 0;
		rectangle[i].envVol = 0;
		rectangle[i].fixedEnvelope = false;
		rectangle[i].freq = 0;
		rectangle[i].freqLimit = 0;
		rectangle[i].holdNote = false;
		rectangle[i].phaseacc = 0;
		rectangle[i].sweepPhase = 0;
		rectangle[i].outputVol = 0;
		rectangle[i].sweepDecay = 0;
		rectangle[i].sweepInc = false;
		rectangle[i].sweepOn = false;
		rectangle[i].sweepShifts = 0;
		rectangle[i].volume = 0;
		rectangle[i].regs[0] = 0;
		rectangle[i].regs[1] = 0;
		rectangle[i].regs[2] = 0;
		rectangle[i].regs[3] = 0;
	}
	// Reset the Triangle Channel
	triangle.enabled = false;
	triangle.vblLength = 0;
	triangle.linearLength = 0;
	triangle.counterStarted = false;
	triangle.writeLatency = 0;
	triangle.freq = 0;
	triangle.outputVol = 0;
	triangle.adder = 0;
	triangle.holdNote = false;
	triangle.phaseacc = 0;
	triangle.writeLatency = 0;
	triangle.regs[0] = 0;
	triangle.regs[1] = 0;
	triangle.regs[2] = 0;
	triangle.regs[3] = 0;
	// Reset the Noise Channel
	noise.enabled = false;
	noise.vblLength = 0;
	noise.curPos = 0;
	noise.envDecay = 0;
	noise.envPhase = 0;
	noise.envVol = 0;
	noise.fixedEnvelope = false;
	noise.freq = 0;
	noise.holdNote = false;
	noise.outputVol = 0;
	noise.phaseAcc = 0;
	noise.shortSample = false;
	noise.volume = 0;
	noise.regs[0] = 0;
	noise.regs[1] = 0;
	noise.regs[2] = 0;
	noise.regs[3] = 0;
	// Reset the Noise Channel
	dmc.regs[0] = 0;
	dmc.regs[1] = 0;
	dmc.regs[2] = 0;
	dmc.regs[3] = 0;
	dmc.enabled = false;
	dmc.freq = 0;
	dmc.phaseacc = 0;
	dmc.outputVol = 0;
	dmc.address = 0;
	dmc.cachedAddr = 0;
	dmc.dmaLength = 0;
	dmc.cachedDMALength = 0;
	dmc.curByte = 0;
	dmc.looping = false;
	dmc.irqGen = false;
	dmc.irqOccurred = false;
	apuCntRate = 5;
}


/**
 * <P>
 * Write to a specified address mapped to the APU.
 * </P>
 */
void apuWrite(int address, int value) {
	// Don't Log Messages if no Sound Engine
	if (!soundEngineLoaded || !enableAllSound)
		return;
	// Create a new Sound Message
	Message mess = {address, value};
	// Queue the Message in the Buffer
	enqueue(mess);
}
