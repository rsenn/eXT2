/*------------------- by Marc Poirier  ][  March 2001 -------------------*/

#ifndef __monomaker
#include "monomaker.hpp"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//-----------------------------------------------------------------------------
// initializations & such

Monomaker::Monomaker(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, numParameters)	// 1 program, 2 parameters
{
	fMonomerge = 0.0f;
	fPan = 0.5f;

	setNumInputs(2);	// stereo in
	setNumOutputs(2);	// stereo out
	setUniqueID((int)"mono");	// identify
	canMono();	// it's okay to feed both inputs with the same signal
	canProcessReplacing();	// supports both accumulating and replacing output
	noTail();	// there is no audio output when the audio input is silence

	strcpy(programName, "let's merge");	// default program name
}

//-----------------------------------------------------------------------------------------
Monomaker::~Monomaker()
{
	// nud
}

//-----------------------------------------------------------------------------------------
// Destroy FX infos

long Monomaker::getVendorVersion() {
	return 101; }

bool Monomaker::getErrorText(char *text) {
	strcpy (text, "Mono kills the music.  What were you thinking?");	// max 256 char
	return true; }

bool Monomaker::getEffectName(char *name) {
	strcpy (name, "Monomaker (stereo)");	// name max 32 char
	return true; }

bool Monomaker::getVendorString(char *text) {
	strcpy (text, "Destroy FX");	// a string identifying the vendor (max 64 char)
	return true; }

bool Monomaker::getProductString(char *text) {
	// a string identifying the product name (max 64 char)
	strcpy (text, "Super Destroy FX bipolar VST plugin pack");
	return true; }

//-----------------------------------------------------------------------------------------
void Monomaker::setProgramName(char *name)
{
	strcpy(programName, name);
}

//-----------------------------------------------------------------------------------------
void Monomaker::getProgramName(char *name)
{
	strcpy(name, programName);
}

//-----------------------------------------------------------------------------------------
void Monomaker::setParameter(long index, float value)
{
	switch (index)
	{
		case kMonomerge : fMonomerge = value;		break;
		case kPan       : fPan = value;		break;
	}
}

//-----------------------------------------------------------------------------------------
float Monomaker::getParameter(long index)
{
	switch (index)
	{
		default:
		case kMonomerge : return fMonomerge;
		case kPan       : return fPan;
	}
}

//-----------------------------------------------------------------------------------------
// titles of each parameter

void Monomaker::getParameterName(long index, char *label)
{
	switch (index)
	{
		case kMonomerge : strcpy(label, "monomix");	break;
		case kPan       : strcpy(label, "pan");		break;
	}
}

//-----------------------------------------------------------------------------------------
// numerical display of each parameter's gradiations

void Monomaker::getParameterDisplay(long index, char *text)
{
	switch (index)
	{
		case kMonomerge :
			sprintf(text, "%.3f", fMonomerge);
			break;
		case kPan       :
			sprintf(text, "%.3f", (fPan*2.0f)-1.0f);
			break;
	}
}

//-----------------------------------------------------------------------------------------
// unit of measure for each parameter

void Monomaker::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case kMonomerge :  strcpy(label, "how much");	break;
		case kPan       :  strcpy(label, "how much");	break;
	}
}

//-----------------------------------------------------------------------------------------
void Monomaker::process(float **inputs, float **outputs, long sampleFrames)
{
  float in1, in2, out1, out2;
  long samplecount;


	for (samplecount=0; (samplecount < sampleFrames); samplecount++)
	{
		// store the input values into short-named variables for shorter math below
		in1 = inputs[0][samplecount];
		in2 = inputs[1][samplecount];

		// this is the monomerging stuff
		out1 = (in1 * (1.0f-(fMonomerge*0.5f))) + (in2 * fMonomerge*0.5f);
		out2 = (in2 * (1.0f-(fMonomerge*0.5f))) + (in1 * fMonomerge*0.5f);

		//this is the panning stuff
		// when fPan > 0.5, then we are panning to the right
		if (fPan > 0.5f)
		{
			outputs[0][samplecount] += out1 * (1.0f-fPan)*2.0f;
			outputs[1][samplecount] += out2 + (out1 * (fPan-0.5f)*2.0f);
		}
		// otherwise we are panning to the left
		else
		{
			outputs[0][samplecount] += out1 + (out2 * (0.5f-fPan)*2.0f);
			outputs[1][samplecount] += out2 * fPan*2.0f;
		}
	}
}

//-----------------------------------------------------------------------------------------
void Monomaker::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
  float in1, in2, out1, out2;
  long samplecount;


	for (samplecount=0; (samplecount < sampleFrames); samplecount++)
	{
		// store the input values into short-named variables for shorter math below
		in1 = inputs[0][samplecount];
		in2 = inputs[1][samplecount];

		// this is the monomerging stuff
		out1 = (in1 * (1.0f-(fMonomerge*0.5f))) + (in2 * fMonomerge*0.5f);
		out2 = (in2 * (1.0f-(fMonomerge*0.5f))) + (in1 * fMonomerge*0.5f);

		//this is the panning stuff
		// when fPan > 0.5, then we are panning to the right
		if (fPan > 0.5f)
		{
			outputs[0][samplecount] = out1 * (1.0f-fPan)*2.0f;
			outputs[1][samplecount] = out2 + (out1 * (fPan-0.5f)*2.0f);
		}
		// otherwise we are panning to the left
		else
		{
			outputs[0][samplecount] = out1 + (out2 * (0.5f-fPan)*2.0f);
			outputs[1][samplecount] = out2 * fPan*2.0f;
		}
	}
}

//-----------------------------------------------------------------------------------------
// this tells the host the plugin can do, which is nothing special in this case

long Monomaker::canDo(char* text)
{
	if (strcmp(text, "plugAsChannelInsert") == 0)
		return 1;
	if (strcmp(text, "plugAsSend") == 0)
		return 1;
	if (strcmp(text, "mixDryWet") == 0)
		return 1;
	if (strcmp(text, "1in1out") == 0)
		return 1;
	if (strcmp(text, "1in2out") == 0)
		return 1;
	if (strcmp(text, "2in1out") == 0)
		return 1;
	if (strcmp(text, "2in2out") == 0)
		return 1;

	return -1;	// explicitly can't do; 0 => don't know
}
