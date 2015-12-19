<<<<<<< HEAD
/*-----------------------------------------------------------------------------

(C)2003 Marko My��en / CMT
A simple bitcrusher plugin with pre-amp gain, bit-depth and downsampling controls
	v1.00 Initial release with gui
	v1.01 Added 3 distortion types and selector switches for them

Thanks to:
	Ossi Honkanen / Exile for gfx
	Urs Heckermann for coding the radiobutton-group control widget!
	Vesa Norilo for valueble debug-information!-D
-----------------------------------------------------------------------------*/

#include "Bitcrusher.hpp"
#include <math.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
Bitcrusher::Bitcrusher(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, kNumParams)	// 1 program, 1 parameter only
{
	// Default positions for the faders and knobs
	fDistortionType = 0.0f;	// Default distortion function (clipper)
	fGain = 1.0f;			// No extra gain by default
	fBitDepth = 0.0f;		// Start with full 24 bits
	fSampleRate = 1.0f;		// Start with full samplerate

	// Init some internal variables
	hold_left = hold_right = 0.f;
	downsampling_pointer = 0;

	// VST parameters
	canMono();				// Works with mono-input too
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID((int)"Bits");	// identify
	canProcessReplacing();	// supports both accumulating and replacing output
	strcpy(programName, "Bitcrusher VST");	// default program name
}

//-----------------------------------------------------------------------------------------
Bitcrusher::~Bitcrusher()
{
	// nothing to do here
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::setProgramName(char *name)
{
	strcpy(programName, name);
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::getProgramName(char *name)
{
	strcpy(name, programName);
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::setParameter (long index, float value)
{
	switch (index)
	{
		case	kDistortionType:fDistortionType = value;break;
		case	kGain	:		fGain = value;			break;
		case	kBitDepth:		fBitDepth = value;		break;
		case	kSampleRate:	fSampleRate = value;	break;
	}
}

//------------------------------------------------------------------------
float Bitcrusher::getParameter (long index)
{
	float v = 0;

	switch (index)
	{
		case kDistortionType:	v = fDistortionType;	break;
		case kGain	:			v = fGain;				break;
		case kBitDepth:			v = fBitDepth;			break;
		case kSampleRate:		v = fSampleRate;		break;
	}
	return v;
}

//------------------------------------------------------------------------
void Bitcrusher::getParameterName (long index, char *label)
{
	switch (index)
	{
		case kDistortionType:	strcpy (label, "Dist type");	break;
		case kGain:				strcpy (label, "Gain");			break;
		case kBitDepth:			strcpy (label, "Bit-depth");	break;
		case kSampleRate:		strcpy (label, "Downsampling");	break;
	}
}

//------------------------------------------------------------------------
void Bitcrusher::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		// Divide the slider area to three equal portitions
		case kDistortionType:
			if(fDistortionType < 0.33)
				sprintf(text, "%i", 1, text);
			if(fDistortionType >= 0.33 && fDistortionType <= 0.66 )
				sprintf(text, "%i", 2, text);
			if(fDistortionType > 0.66)
				sprintf(text, "%i", 3, text);
		break;

		case kGain:				sprintf(text, "%f", 20.*log10((-16.f * fGain + 17.f)), text);	break;
		case kBitDepth:			sprintf(text,"%i",(int)(-23.f * fBitDepth + 24.f));				break;
		case kSampleRate:		sprintf(text,"%i",(int)(-39.f * fSampleRate + 40.f));			break;
	}
}

//------------------------------------------------------------------------
void Bitcrusher::getParameterLabel (long index, char *label)
{
	switch (index)
	{
		case kDistortionType:	strcpy (label, "");		break;
		case kGain:				strcpy (label, "dB");	break;
		case kBitDepth:			strcpy (label, "Bits");	break;
		case kSampleRate:		strcpy (label, "x");	break;
	}
}

void Bitcrusher::process(float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    while(--sampleFrames >= 0)
    {
        (*out1++) = (*in1++);    // No processing!!
        (*out2++) = (*in2++);
    }
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

	// Precalc some variables
	float bit_depth = (float)pow(2, -23*fBitDepth + 24);
	float inv_bit_depth = 1.f / bit_depth;

	// 0-24dBs of extra gain by default
	float preamp = -16.f*fGain + 17.f;

	// Distortion switch #1 (clipper)
	if(fDistortionType < 0.33)
	{
		while(--sampleFrames >= 0)
		{
			// Pre-processing gain
			float temp_left = preamp*(*in1++);
			float temp_right = preamp*(*in2++);

			// Distortion using hard clipping
			if( temp_left > 1.0)
				temp_left = 1.0;
			if( temp_right > 1.0)
				temp_right = 1.0;
			if( temp_left < -1.0)
				temp_left = -1.0;
			if( temp_right < -1.0)
				temp_right = -1.0;

			// Downsampling using sample and hold
			if(downsampling_pointer < -39*fSampleRate + 39)
				downsampling_pointer++;
			else
			{
				hold_left = temp_left;
				hold_right = temp_right;
				downsampling_pointer = 0;
			}

			// Bitcrushing
			(*out1++) = ((int)(hold_left*bit_depth)) * inv_bit_depth;
			(*out2++) = ((int)(hold_right*bit_depth)) * inv_bit_depth;
		}
	}

	// Distortion switch #2 (tanh)
	if(fDistortionType >= 0.33 && fDistortionType <= 0.66 )
	{
		while(--sampleFrames >= 0)
		{
			// Pre-processing gain
			float temp_left = preamp*(*in1++);
			float temp_right = preamp*(*in2++);

			// Soft distortion using hyperbolic tan-function
			temp_left = (float)tanh(temp_left);
			temp_right = (float)tanh(temp_right);

			// Downsampling using sample and hold
			if(downsampling_pointer < -39*fSampleRate + 39)
				downsampling_pointer++;
			else
			{
				hold_left = temp_left;
				hold_right = temp_right;
				downsampling_pointer = 0;
			}

			// Bitcrushing
			(*out1++) = ((int)(hold_left*bit_depth)) * inv_bit_depth;
			(*out2++) = ((int)(hold_right*bit_depth)) * inv_bit_depth;
		}
	}

	// Distortion switch #3 (sin)
	if(fDistortionType > 0.66)
	{
		while(--sampleFrames >= 0)
		{
			// Pre-processing gain
			float temp_left = preamp*(*in1++);
			float temp_right = preamp*(*in2++);

			temp_left = (float)sin(temp_left);
			temp_right = (float)sin(temp_right);

			if(downsampling_pointer < -39*fSampleRate + 39)
				downsampling_pointer++;
			else
			{
				hold_left = temp_left;
				hold_right = temp_right;
				downsampling_pointer = 0;
			}

			// Bitcrushing
			(*out1++) = ((int)(hold_left*bit_depth)) * inv_bit_depth;
			(*out2++) = ((int)(hold_right*bit_depth)) * inv_bit_depth;
		}
	}
}
=======
/*-----------------------------------------------------------------------------

(C)2003 Marko My��en / CMT
A simple bitcrusher plugin with pre-amp gain, bit-depth and downsampling controls
	v1.00 Initial release with gui
	v1.01 Added 3 distortion types and selector switches for them

Thanks to:
	Ossi Honkanen / Exile for gfx
	Urs Heckermann for coding the radiobutton-group control widget!
	Vesa Norilo for valueble debug-information!-D
-----------------------------------------------------------------------------*/

#include "Bitcrusher.hpp"
#include <math.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
Bitcrusher::Bitcrusher(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, kNumParams)	// 1 program, 1 parameter only
{
	// Default positions for the faders and knobs
	fDistortionType = 0.0f;	// Default distortion function (clipper)
	fGain = 1.0f;			// No extra gain by default
	fBitDepth = 0.0f;		// Start with full 24 bits
	fSampleRate = 1.0f;		// Start with full samplerate

	// Init some internal variables
	hold_left = hold_right = 0.f;
	downsampling_pointer = 0;

	// VST parameters
	canMono();				// Works with mono-input too
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID((int)"Bits");	// identify
	canProcessReplacing();	// supports both accumulating and replacing output
	strcpy(programName, "Bitcrusher VST");	// default program name
}

//-----------------------------------------------------------------------------------------
Bitcrusher::~Bitcrusher()
{
	// nothing to do here
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::setProgramName(char *name)
{
	strcpy(programName, name);
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::getProgramName(char *name)
{
	strcpy(name, programName);
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::setParameter (long index, float value)
{
	switch (index)
	{
		case	kDistortionType:fDistortionType = value;break;
		case	kGain	:		fGain = value;			break;
		case	kBitDepth:		fBitDepth = value;		break;
		case	kSampleRate:	fSampleRate = value;	break;
	}
}

//------------------------------------------------------------------------
float Bitcrusher::getParameter (long index)
{
	float v = 0;

	switch (index)
	{
		case kDistortionType:	v = fDistortionType;	break;
		case kGain	:			v = fGain;				break;
		case kBitDepth:			v = fBitDepth;			break;
		case kSampleRate:		v = fSampleRate;		break;
	}
	return v;
}

//------------------------------------------------------------------------
void Bitcrusher::getParameterName (long index, char *label)
{
	switch (index)
	{
		case kDistortionType:	strcpy (label, "Dist type");	break;
		case kGain:				strcpy (label, "Gain");			break;
		case kBitDepth:			strcpy (label, "Bit-depth");	break;
		case kSampleRate:		strcpy (label, "Downsampling");	break;
	}
}

//------------------------------------------------------------------------
void Bitcrusher::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		// Divide the slider area to three equal portitions
		case kDistortionType:
			if(fDistortionType < 0.33)
				sprintf(text, "%i", 1, text);
			if(fDistortionType >= 0.33 && fDistortionType <= 0.66 )
				sprintf(text, "%i", 2, text);
			if(fDistortionType > 0.66)
				sprintf(text, "%i", 3, text);
		break;

		case kGain:				sprintf(text, "%f", 20.*log10((-16.f * fGain + 17.f)), text);	break;
		case kBitDepth:			sprintf(text,"%i",(int)(-23.f * fBitDepth + 24.f));				break;
		case kSampleRate:		sprintf(text,"%i",(int)(-39.f * fSampleRate + 40.f));			break;
	}
}

//------------------------------------------------------------------------
void Bitcrusher::getParameterLabel (long index, char *label)
{
	switch (index)
	{
		case kDistortionType:	strcpy (label, "");		break;
		case kGain:				strcpy (label, "dB");	break;
		case kBitDepth:			strcpy (label, "Bits");	break;
		case kSampleRate:		strcpy (label, "x");	break;
	}
}

void Bitcrusher::process(float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    while(--sampleFrames >= 0)
    {
        (*out1++) = (*in1++);    // No processing!!
        (*out2++) = (*in2++);
    }
}

//-----------------------------------------------------------------------------------------
void Bitcrusher::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

	// Precalc some variables
	float bit_depth = (float)pow(2, -23*fBitDepth + 24);
	float inv_bit_depth = 1.f / bit_depth;

	// 0-24dBs of extra gain by default
	float preamp = -16.f*fGain + 17.f;

	// Distortion switch #1 (clipper)
	if(fDistortionType < 0.33)
	{
		while(--sampleFrames >= 0)
		{
			// Pre-processing gain
			float temp_left = preamp*(*in1++);
			float temp_right = preamp*(*in2++);

			// Distortion using hard clipping
			if( temp_left > 1.0)
				temp_left = 1.0;
			if( temp_right > 1.0)
				temp_right = 1.0;
			if( temp_left < -1.0)
				temp_left = -1.0;
			if( temp_right < -1.0)
				temp_right = -1.0;

			// Downsampling using sample and hold
			if(downsampling_pointer < -39*fSampleRate + 39)
				downsampling_pointer++;
			else
			{
				hold_left = temp_left;
				hold_right = temp_right;
				downsampling_pointer = 0;
			}

			// Bitcrushing
			(*out1++) = ((int)(hold_left*bit_depth)) * inv_bit_depth;
			(*out2++) = ((int)(hold_right*bit_depth)) * inv_bit_depth;
		}
	}

	// Distortion switch #2 (tanh)
	if(fDistortionType >= 0.33 && fDistortionType <= 0.66 )
	{
		while(--sampleFrames >= 0)
		{
			// Pre-processing gain
			float temp_left = preamp*(*in1++);
			float temp_right = preamp*(*in2++);

			// Soft distortion using hyperbolic tan-function
			temp_left = (float)tanh(temp_left);
			temp_right = (float)tanh(temp_right);

			// Downsampling using sample and hold
			if(downsampling_pointer < -39*fSampleRate + 39)
				downsampling_pointer++;
			else
			{
				hold_left = temp_left;
				hold_right = temp_right;
				downsampling_pointer = 0;
			}

			// Bitcrushing
			(*out1++) = ((int)(hold_left*bit_depth)) * inv_bit_depth;
			(*out2++) = ((int)(hold_right*bit_depth)) * inv_bit_depth;
		}
	}

	// Distortion switch #3 (sin)
	if(fDistortionType > 0.66)
	{
		while(--sampleFrames >= 0)
		{
			// Pre-processing gain
			float temp_left = preamp*(*in1++);
			float temp_right = preamp*(*in2++);

			temp_left = (float)sin(temp_left);
			temp_right = (float)sin(temp_right);

			if(downsampling_pointer < -39*fSampleRate + 39)
				downsampling_pointer++;
			else
			{
				hold_left = temp_left;
				hold_right = temp_right;
				downsampling_pointer = 0;
			}

			// Bitcrushing
			(*out1++) = ((int)(hold_left*bit_depth)) * inv_bit_depth;
			(*out2++) = ((int)(hold_right*bit_depth)) * inv_bit_depth;
		}
	}
}
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
