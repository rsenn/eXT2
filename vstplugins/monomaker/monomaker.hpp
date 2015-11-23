/*------------------- by Marc Poirier  ][  March 2001 -------------------*/

#ifndef __MONOMAKER_H
#define __MONOMAKER_H

#include <audioeffectx.h>


//----------------------------------------------------------------------------- 
// these are the plugin parameters:
enum
{
	kMonomerge,
	kPan,

	numParameters
};



//----------------------------------------------------------------------------- 

class Monomaker : public AudioEffectX
{
public:
	Monomaker(audioMasterCallback audioMaster);
	~Monomaker();

	virtual void process(float **inputs, float **outputs, long sampleFrames);
	virtual void processReplacing(float **inputs, float **outputs, long sampleFrames);

	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);

	virtual void setParameter(long index, float value);
	virtual float getParameter(long index);
	virtual void getParameterName(long index, char *text);
	virtual void getParameterDisplay(long index, char *text);
	virtual void getParameterLabel(long index, char *label);

	virtual bool getEffectName(char *name);
	virtual long getVendorVersion();
	virtual bool getErrorText(char *text);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);

	virtual long canDo(char* text);

// this is my stuff:
protected:
	float fMonomerge, fPan;	// the parameters
	char programName[32];
};

#endif
