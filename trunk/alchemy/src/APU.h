#ifndef APU_H_
#define APU_H_

typedef struct{

int regs[4];
boolean enabled;
boolean looping;
boolean irqGen;
boolean irqOccurred;

int freq;
int phaseacc;
int outputVol;
int address;
int cachedAddr;
int dmaLength;
int cachedDMALength;
int curByte;

//void reload() {
//	address = cachedAddr;
//	dmaLength = cachedDMALength;
//	irqOccurred;
//}
} DMC;

typedef struct{

int regs[4];
boolean enabled;
boolean holdNote;
boolean counterStarted;

int freq;
int linearLength;
int outputVol;
int adder;
int phaseacc;
int writeLatency;
int vblLength;

} Triangle;

typedef struct{

int regs[4];
boolean enabled;
boolean fixedEnvelope;
boolean holdNote;
boolean sweepInc;
boolean sweepOn;

int freq;
int adder;
int dutyFlip;
int envDecay;
int envPhase;
int envVol;
int freqLimit;
int phaseacc;
int sweepPhase;
int outputVol;
int sweepDecay;
int sweepShifts;
int vblLength;
int volume;

} Rectangle;

typedef struct{

int regs[4];
boolean enabled;
boolean holdNote;
boolean fixedEnvelope;
boolean shortSample;

int freq;
int curPos;
int envDecay;
int envPhase;
int envVol;
int outputVol;
int phaseAcc;
int vblLength;
int volume;

} Noise;

typedef struct{

int address;
int value;

}Message;

int apuRead(int address);
void apuWrite(int address, int value);
void regWrite(int address, int value);
void reset();
void refreshAPU();
void init(int SampleRate, int RefreshRate);

int processTriangle(Triangle chan);
int processDMC(DMC chan);
int processRectangle(Rectangle chan);
int processNoise(Noise chan);

void genRandomSamples(int count);
void buildLUTs(int numSamples);
int decayVolume(int vol);
void enqueue(Message message);
Message dequeue();

void playAPUSound(short* buf, int samples);

#endif /* APU_H_ */
