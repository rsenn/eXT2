//	ParameterFilter.h - A filter to smooth parameter changes, taken from
//						music dsp.org:
//						http://www.musicdsp.org/archive.php?classid=3#28
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

#ifndef PARAMETERFILTER_H_
#define PARAMETERFILTER_H_

///	A simple filter class used to smooth parameter changes.
/*!
	This code was taken from musicdsp.org, specifically:
	http://www.musicdsp.org/archive.php?classid=3#28
 */
class ParameterFilter
{
  public:
	///	Constructor.
	/*!
		\param scale How smooth the change is?
	 */
	ParameterFilter(float samplerate, double scale);
	///	Destructor.
	~ParameterFilter() {};

	///	Returns the next sample.
	float GetSample(float inval);

	///	Sets the scale parameter.
	void SetScale(double samples);	//takes a value 0->1
	///	Sets the smoothness.
	void SetSmooth(double val);		//ditto
	///	Sets the samplerate.
	void SetSamplerate(float samplerate);
  private:
	//--parameters--
	double Scale;
	double Smoothness;
	float Samplerate;

	//--precalc variables--
	double A,
		   B;
	double ACoeff,
		   BCoeff,
		   CCoeff;
	double MasterGain;
	double AGain,
		   BGain,
		   CGain;

	//--runtime variables--
	double AReg, BReg, CReg;
};

#endif
