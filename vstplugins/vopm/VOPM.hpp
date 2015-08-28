/*-----------------------------------------------------------------------------

© 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/

/*

*/

#ifndef __VOPM__
#define __VOPM__

#include <string.h>

#ifndef __AudioEffectX__
#include "audioeffectx.h"
#endif

#ifndef VstIntPtr
  #define VstIntPtr long
#endif

//const double Pi = 3.141592653f;

enum
{
	kNumPrograms = 128,
	kNumOutputs = 2,

	kCon=0,
	kFL,
	kAMS,
	kPMS,
	kNFQ,
	kNE,
	kSPD,
	kAMD,
	kPMD,
	kWF,
	kOpMsk,

//OP M1
	kM1TL,

	kM1AR,
	kM1D1R,
	kM1D1L,
	kM1D2R,
	kM1RR,
	kM1KS,
	kM1MUL,
	kM1DT1,
	kM1DT2,
	kM1F_AME,

//OP C1
	kC1TL,
	kC1AR,
	kC1D1R,
	kC1D1L,
	kC1D2R,
	kC1RR,
	kC1KS,
	kC1MUL,
	kC1DT1,
	kC1DT2,
	kC1F_AME,
//OP M2
	kM2TL,
	kM2AR,
	kM2D1R,
	kM2D1L,
	kM2D2R,
	kM2RR,
	kM2KS,
	kM2MUL,
	kM2DT1,
	kM2DT2,
	kM2F_AME,
//OP C2
	kC2TL,
	kC2AR,
	kC2D1R,
	kC2D1L,
	kC2D2R,
	kC2RR,
	kC2KS,
	kC2MUL,
	kC2DT1,
	kC2DT2,
	kC2F_AME,
	kNumParams,
	kMskM1,
	kMskC1,
	kMskM2,
	kMskC2,

	kFileLoad,
	kFileSave,
	kWf0,
	kWf1,
	kWf2,
	kWf3,
	kCon0,
	kCon1,
	kCon2,
	kCon3,
	kCon4,
	kCon5,
	kCon6,
	kCon7,
	kHdsw
};


//------------------------------------------------------------------------------------------
class VOPM : public AudioEffectX
{
public:
	VOPM(audioMasterCallback audioMaster);
	~VOPM();
	class Opm *pOPM;
	class OPMDRV *pOPMdrv;
	float fVolume;

	virtual void process(float **inputs, float **outputs, long sampleframes);
	virtual void processReplacing(float **inputs, float **outputs, long sampleframes);
	virtual long processEvents(VstEvents* events);

	virtual void setProgram(long program);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(long index, float value);
	virtual float getParameter(long index);
	virtual void getParameterLabel(long index, char *label);
	virtual void getParameterDisplay(long index, char *text);
	virtual void getParameterName(long index, char *text);
	virtual void setSampleRate(float sampleRate);
	virtual void setBlockSize(long blockSize);
	virtual void resume();

	virtual bool getOutputProperties (long index, VstPinProperties* properties);
//	virtual bool getProgramNameIndexed (long category, long index, char* text);
	virtual bool copyProgram (long destination);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
//	long getVendorVersion () {return 1;}
	virtual VstIntPtr vendorSpecific (long lArg, long lArg2, void* ptrArg, float floatArg);


	virtual long canDo (char* text);
	virtual long getChunk(void** data, bool isPreset = false);	// returns byteSize
	virtual long setChunk(void* data, long byteSize, bool isPreset = false);

	void initProcess();
	
    long dispatcher (long opCode, long index, long value, void *ptr, float opt);	

private:
	int oome;
	unsigned char *pChunk;
	long rate;
	long SampleRate;
	void noteOn(long note, long velocity, long delta);
	long lTblCcLearn[128];
};

#endif
