<<<<<<< HEAD
//	wavefileloader.h - Class for loading .wav files.
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
=======
//	wavefileloader.h - Class for loading .wav files.
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
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
//	--------------------------------------------------------------------------

#ifndef WAVEFILELOADER_H_
#define WAVEFILELOADER_H_
<<<<<<< HEAD

=======

>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
///	.wav file header structure.
typedef struct {
	char  Riff[4];
	long RiffSize;
	char  Wave[4];
	char  Fmt[4];
	long FmtSize;
	short FormatTag;
	short noChannels;
	long Samplerate;
	long AvgBytesPerSec;
	short BlockAlign;
	short BitDepth;
	char  Data[4];
	long DataSize;
	void  *waveformData;
} WAVEFILE;
<<<<<<< HEAD

///	Simple class to load a .wav file.
class wavefileloader
=======

///	Simple class to load a .wav file.
class wavefileloader
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
{
public:

    virtual ~wavefileloader () {}
    
	///	Loads a .wav file, should resample it to the correct samplerate.
	virtual float *LoadWaveFile(char *filename, float samplerate);
<<<<<<< HEAD

	///	Returns the size of the previously-loaded file.
	virtual unsigned long getSize();
  protected:
	unsigned long size;

	///	Alters the samplerate of buffer, from sr_src to sr_dest.
	virtual float *AlterSamplerate(float *buffer, float sr_src, float sr_dest);
	///	Simple linear interpolation.
	virtual float calcInterpValue(float val1, float val2, float index);
};

#endif
=======

	///	Returns the size of the previously-loaded file.
	virtual unsigned long getSize();
  protected:
	unsigned long size;

	///	Alters the samplerate of buffer, from sr_src to sr_dest.
	virtual float *AlterSamplerate(float *buffer, float sr_src, float sr_dest);
	///	Simple linear interpolation.
	virtual float calcInterpValue(float val1, float val2, float index);
};

#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
