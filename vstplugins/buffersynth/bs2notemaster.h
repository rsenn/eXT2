<<<<<<< HEAD
//  bs2notemaster.hpp - Keeps track of how many notes are playing, calls
//					  bs2voice.
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

#ifndef BS2NOTEMASTER_H_
#define BS2NOTEMASTER_H_

#include "bs2voice.h"

#define NUM_VOICES 16	//i.e. 16-note polyphony

///	This class keeps track of the individual bs2voices.
/*!
	Keeps track of which notes are playing etc. Also holds pointers to certain
	buffers which are common to all instances of the bs2voices.
 */
class bs2notemaster
{
  public:
	///	Constructor.
	bs2notemaster(float samplerate);
	///	Destructor
	~bs2notemaster();

	///	Called to indicate a MIDI Note On message has been received.
	/*!
		This will result in the first free bs2voice the notemaster can find
		starting to play (with envelopes set to the start of their Attack
		phase etc.).
	 */
	void NoteOn(int note, float velocity);
	///	Called to indicate a MIDI Note Off message has been received.
	void NoteOff(int note);
	///	Sets the pitch bend of all the bs2voices (whether they're active or not).
	void SetPitchBend(float val);

	///	Writes a sample into buffer 1.
	/*!
		Used when the input's an audio stream.
	 */
	void Write2IPBuffer1(float inval);
	///	Fills buffer 1 with a bunch of samples.
	/*!
		Used when the input's a wave file.
	 */
	void Write2IPBuffer1(float *array);
	///	Writes a sample into buffer 2.
	/*!
		Used when the input's an audio stream.
	 */
	void Write2IPBuffer2(float inval);
	///	Fills buffer 2 with a bunch of samples.
	/*!
		Used when the input's a wave file.
	 */
	void Write2IPBuffer2(float *array);
	///	Sets the indexed parameter of all the bs2voices to val.
	void SetParameter(long index, float val);
	///	Sets the samplerate of all the bs2voices.
	void SetSamplerate(float samplerate);
	///	Sets the current tempo for all the bs2voices.
	/*!
		(this is used for tempo sync stuff)
	 */
	void SetTempo(float tempo);

	///	I can't actually remember what this is for...
	float GetCS() {return Voices[0].GetCS();};
	///	Fills buf with the contents of buffer 1, skipping however many samples are necessary to fit it into a buffer of size size.
	void GetBuffer1(float *buf, int size);
	///	Fills buf with the contents of buffer 2, skipping however many samples are necessary to fit it into a buffer of size size.
	void GetBuffer2(float *buf, int size);
	///	Fills buf with the contents of buffer1.
	void GetBuffer1(float *buf);
	///	Fills buf with the contents of buffer2.
	void GetBuffer2(float *buf);
	///	Called every sample, to get the next sample to send to the host.
	twofloats GetSample(bool barstart);

	///	Our current place in buffer 1 (for writing to it).
	long IPBuff1Index;
	///	Our current place in buffer 2 (for writing to it).
	long IPBuff2Index;
  private:
	float Samplerate;
	///	This is buffer 1, it is shared by all the bs2voices.
	float *IPBuffer1;
	///	This is buffer 2, it is shared by all the bs2voices.
	float *IPBuffer2;

	///	Array of voices (where the actual audio stuff takes place).
	bs2voice *Voices;
	///	What notes correspond to what bs2voices/
	/*!
		i.e. Voices[i] => Notes[i]
	 */
	int Notes[NUM_VOICES];

	///	Indicates whether the indexed voice is currently active or not.
	bool VoiceIsActive[NUM_VOICES];
	///	Used to determine when to update VoiceIsActive.
	/*!
		We only update VoiceIsActive every 5 samples, to save clock
		cycles.  This is probably unnecessary.
	 */
	int VIACountDown;

	///	A sine wave lookup table shared by all the bs2voices.
	float *SineWave;
	///	A tanh lookup table shared by all the bs2voices.
	/*!
		This is used by the envelopes and the filter.
	 */
	float *TanhWave;

	///	Indicates whether we're in polyphonic mode or not.
	bool PolyphonicMode;
};

#endif
=======
//  bs2notemaster.hpp - Keeps track of how many notes are playing, calls
//					  bs2voice.
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

#ifndef BS2NOTEMASTER_H_
#define BS2NOTEMASTER_H_

#include "bs2voice.h"

#define NUM_VOICES 16	//i.e. 16-note polyphony

///	This class keeps track of the individual bs2voices.
/*!
	Keeps track of which notes are playing etc. Also holds pointers to certain
	buffers which are common to all instances of the bs2voices.
 */
class bs2notemaster
{
  public:
	///	Constructor.
	bs2notemaster(float samplerate);
	///	Destructor
	~bs2notemaster();

	///	Called to indicate a MIDI Note On message has been received.
	/*!
		This will result in the first free bs2voice the notemaster can find
		starting to play (with envelopes set to the start of their Attack
		phase etc.).
	 */
	void NoteOn(int note, float velocity);
	///	Called to indicate a MIDI Note Off message has been received.
	void NoteOff(int note);
	///	Sets the pitch bend of all the bs2voices (whether they're active or not).
	void SetPitchBend(float val);

	///	Writes a sample into buffer 1.
	/*!
		Used when the input's an audio stream.
	 */
	void Write2IPBuffer1(float inval);
	///	Fills buffer 1 with a bunch of samples.
	/*!
		Used when the input's a wave file.
	 */
	void Write2IPBuffer1(float *array);
	///	Writes a sample into buffer 2.
	/*!
		Used when the input's an audio stream.
	 */
	void Write2IPBuffer2(float inval);
	///	Fills buffer 2 with a bunch of samples.
	/*!
		Used when the input's a wave file.
	 */
	void Write2IPBuffer2(float *array);
	///	Sets the indexed parameter of all the bs2voices to val.
	void SetParameter(long index, float val);
	///	Sets the samplerate of all the bs2voices.
	void SetSamplerate(float samplerate);
	///	Sets the current tempo for all the bs2voices.
	/*!
		(this is used for tempo sync stuff)
	 */
	void SetTempo(float tempo);

	///	I can't actually remember what this is for...
	float GetCS() {return Voices[0].GetCS();};
	///	Fills buf with the contents of buffer 1, skipping however many samples are necessary to fit it into a buffer of size size.
	void GetBuffer1(float *buf, int size);
	///	Fills buf with the contents of buffer 2, skipping however many samples are necessary to fit it into a buffer of size size.
	void GetBuffer2(float *buf, int size);
	///	Fills buf with the contents of buffer1.
	void GetBuffer1(float *buf);
	///	Fills buf with the contents of buffer2.
	void GetBuffer2(float *buf);
	///	Called every sample, to get the next sample to send to the host.
	twofloats GetSample(bool barstart);

	///	Our current place in buffer 1 (for writing to it).
	long IPBuff1Index;
	///	Our current place in buffer 2 (for writing to it).
	long IPBuff2Index;
  private:
	float Samplerate;
	///	This is buffer 1, it is shared by all the bs2voices.
	float *IPBuffer1;
	///	This is buffer 2, it is shared by all the bs2voices.
	float *IPBuffer2;

	///	Array of voices (where the actual audio stuff takes place).
	bs2voice *Voices;
	///	What notes correspond to what bs2voices/
	/*!
		i.e. Voices[i] => Notes[i]
	 */
	int Notes[NUM_VOICES];

	///	Indicates whether the indexed voice is currently active or not.
	bool VoiceIsActive[NUM_VOICES];
	///	Used to determine when to update VoiceIsActive.
	/*!
		We only update VoiceIsActive every 5 samples, to save clock
		cycles.  This is probably unnecessary.
	 */
	int VIACountDown;

	///	A sine wave lookup table shared by all the bs2voices.
	float *SineWave;
	///	A tanh lookup table shared by all the bs2voices.
	/*!
		This is used by the envelopes and the filter.
	 */
	float *TanhWave;

	///	Indicates whether we're in polyphonic mode or not.
	bool PolyphonicMode;
};

#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
