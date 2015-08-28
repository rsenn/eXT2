//	bs2wfl.cpp - Buffer Synth 2 wave file loader, includes 'stretch' option.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include "bs2wfl.h"

#ifndef _INTERP_FUNCTION_2_
#define _INTERP_FUNCTION_2_
inline float interpolate(float val1, float val2, float index)
{
	float outval, index_fract;

	index_fract = index - (float)((int)(index));
	outval = val1 + ((val2 - val1)*index_fract);

	return outval;
}
#endif

float *bs2wfl::LoadWaveFile(char *filename, float samplerate, float Stretch, unsigned long stretchSize)
{
	unsigned long i;
	float *outval;
	float increment, index;
	float *newbuf;
	unsigned long temp;

	outval = wavefileloader::LoadWaveFile (filename, samplerate);

	if(Stretch > 0.5f)
	{
		if(size == stretchSize)
			return outval;

		newbuf = new float[stretchSize];

		index = 0.0f;
		increment = (float)size/(float)stretchSize;

		for(i=0;i<stretchSize;i++)
		{
			temp = (int) index;
			newbuf[i] = interpolate(outval[temp], outval[temp+1], index);

			index += increment;
		}

		delete [] outval;

		size = stretchSize;

		return newbuf;
	}
	else
		return outval;
}
