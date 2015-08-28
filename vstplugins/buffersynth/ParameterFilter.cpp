//	ParameterFilter.cpp - A filter to smooth parameter changes, taken from
//						  music dsp.org:
//						  http://www.musicdsp.org/archive.php?classid=3#28
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

#include "ParameterFilter.h"
#include <math.h>

ParameterFilter::ParameterFilter(float samplerate, double scale)
{
	Samplerate = samplerate;
	Scale = scale * Samplerate;
	Smoothness = 0.999;

	A = 1.0-(2.4/Scale);
	B = Smoothness;
	ACoeff = A;
	BCoeff = A*B;
	CCoeff = A*B*B;
	MasterGain = 1.0/(-1.0/(log(A)+2.0*log(B))+2.0/(log(A)+log(B))-1.0/log(A));
	AGain = MasterGain;
	BGain = MasterGain*(log(A*B*B)*(log(A)-log(A*B))/((log(A*B*B)-log(A*B))*log(A*B))-log(A)/log(A*B));
	CGain = MasterGain*(-(log(A)-log(A*B))/(log(A*B*B)-log(A*B)));

	AReg = 0.0;
	BReg = 0.0;
	CReg = 0.0;
}

float ParameterFilter::GetSample(float inval)
{
	double retval;

	AReg = ACoeff*AReg + (double)inval;
	BReg = BCoeff*BReg + (double)inval;
	CReg = CCoeff*CReg + (double)inval;

	retval = AGain*AReg + BGain*BReg + CGain*CReg;

	if(retval > 1.0)
		retval = 1.0;
	else if(retval < 0.0)
		retval = 0.0;

	return (float)retval;
}

void ParameterFilter::SetScale(double samples)
{
	Scale = samples * Samplerate;
	Smoothness = 0.999;

	A = 1.0-(2.4/Scale);
	B = Smoothness;
	ACoeff = A;
	BCoeff = A*B;
	CCoeff = A*B*B;
	MasterGain = 1.0/(-1.0/(log(A)+2.0*log(B))+2.0/(log(A)+log(B))-1.0/log(A));
	AGain = MasterGain;
	BGain = MasterGain*(log(A*B*B)*(log(A)-log(A*B))/((log(A*B*B)-log(A*B))*log(A*B))-log(A)/log(A*B));
	CGain = MasterGain*(-(log(A)-log(A*B))/(log(A*B*B)-log(A*B)));
}

void ParameterFilter::SetSmooth(double val)
{
	Smoothness = val;

	B = Smoothness;
	BCoeff = A*B;
	CCoeff = A*B*B;
	MasterGain = 1.0/(-1.0/(log(A)+2.0*log(B))+2.0/(log(A)+log(B))-1.0/log(A));
	AGain = MasterGain;
	BGain = MasterGain*(log(A*B*B)*(log(A)-log(A*B))/((log(A*B*B)-log(A*B))*log(A*B))-log(A)/log(A*B));
	CGain = MasterGain*(-(log(A)-log(A*B))/(log(A*B*B)-log(A*B)));
}

void ParameterFilter::SetSamplerate(float samplerate)
{
	Scale /= Samplerate;
	Samplerate = samplerate;
	Scale *= Samplerate;

	A = 1.0-(2.4/Scale);
	B = Smoothness;
	ACoeff = A;
	BCoeff = A*B;
	CCoeff = A*B*B;
	MasterGain = 1.0/(-1.0/(log(A)+2.0*log(B))+2.0/(log(A)+log(B))-1.0/log(A));
	AGain = MasterGain;
	BGain = MasterGain*(log(A*B*B)*(log(A)-log(A*B))/((log(A*B*B)-log(A*B))*log(A*B))-log(A)/log(A*B));
	CGain = MasterGain*(-(log(A)-log(A*B))/(log(A*B*B)-log(A*B)));
}
