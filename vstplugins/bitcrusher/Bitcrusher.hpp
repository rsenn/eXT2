/*-----------------------------------------------------------------------------

© 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/
#ifndef __BITCRUSHER_H
#define __BITCRUSHER_H

#include "audioeffectx.h"

enum
{
	kDistortionType,
	kGain,
	kBitDepth,
	kSampleRate,
	
	kNumParams
};

class Bitcrusher : public AudioEffectX
{
public:
	Bitcrusher(audioMasterCallback audioMaster);
	~Bitcrusher();

	virtual void process(float **inputs, float **outputs, long sampleFrames);
	virtual void processReplacing(float **inputs, float **outputs, long sampleFrames);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(long index, float value);
	virtual float getParameter(long index);
	virtual void getParameterLabel(long index, char *label);
	virtual void getParameterDisplay(long index, char *text);
	virtual void getParameterName(long index, char *text);

protected:
	float	fDistortionType, fGain, fBitDepth, fSampleRate;
	float	bitcrush_left, bitcrush_right, hold_left, hold_right;
	int		downsampling_pointer;
	char programName[32];
};

#endif
