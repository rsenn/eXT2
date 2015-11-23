//	bs2notemaster.cpp - Keeps track of how many notes are playing, calls
//						bs2voice.
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

#include <math.h>
#include "bs2notemaster.h"

//----------------------------------------------------------------------------
bs2notemaster::bs2notemaster(float samplerate)
{
	long i;

	Samplerate = samplerate;

	IPBuff1Index = 0;	//so that the output's only 1 sample behind the input (rather than 44100)
	IPBuff2Index = 0;
	IPBuffer1 = 0;
	IPBuffer2 = 0;
	SineWave = 0;
	TanhWave = 0;
	Voices = 0;

	IPBuffer1 = new float[BUFFERSIZE];
	IPBuffer2 = new float[BUFFERSIZE];

	SineWave = new float[TABLE_SIZE];
	TanhWave = new float[TABLE_SIZE];

	for(i=0;i<BUFFERSIZE;i++)
	{
		IPBuffer1[i] = 0.0f;
		IPBuffer2[i] = 0.0f;
	}
	for(i=0;i<TABLE_SIZE;i++)
	{
		SineWave[i] = (float)sin((i*TWO_PI)/(float)TABLE_SIZE);
		TanhWave[i] = (float)tanh(((i*TWO_PI)/(float)TABLE_SIZE)-NDC_PI);
	}

	Voices = new bs2voice[NUM_VOICES];
	for(i=0;i<NUM_VOICES;i++)
	{
		Voices[i].Create(this, i, Samplerate, IPBuffer1, IPBuffer2, TanhWave, SineWave);
		VoiceIsActive[i] = false;
	}
	Voices[0].setFirstVoice();
	VIACountDown = 5;

	PolyphonicMode = false;
}

//----------------------------------------------------------------------------
bs2notemaster::~bs2notemaster()
{
	if(Voices)
		delete [] Voices;
	if(TanhWave)
		delete [] TanhWave;
	if(SineWave)
		delete [] SineWave;
	if(IPBuffer2)
		delete [] IPBuffer2;
	if(IPBuffer1)
		delete [] IPBuffer1;
}

//----------------------------------------------------------------------------
void bs2notemaster::NoteOn(int note, float velocity)
{
	int i;
	bool test = false;

	if(velocity == 0)
		return;

	if(!PolyphonicMode)
	{
		Voices[0].NoteOff();
		Voices[0].NoteOn(note, velocity);
		Notes[0] = note;
		return;
	}
	
	for(i=0;i<NUM_VOICES;i++)
	{
		if(Notes[i] == note)
			return;
	}

	for(i=0;i<NUM_VOICES;i++)
	{
		if(!Voices[i].GetIsActive())
		{
			Voices[i].NoteOn(note, velocity);
			Notes[i] = note;
			break;
		}
	}
}

//----------------------------------------------------------------------------
void bs2notemaster::NoteOff(int note)
{
	int i;

	if(!PolyphonicMode)
	{
		if(Notes[0] == note)
		{
			Voices[0].NoteOff();
			Notes[0] = -1;
		}
		return;
	}

	for(i=0;i<NUM_VOICES;i++)
	{
		if(Notes[i] == note)
		{
			Voices[i].NoteOff();
			Notes[i] = -1;
			break;
		}
	}
}

//----------------------------------------------------------------------------
void bs2notemaster::SetPitchBend(float val)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
	{
		Voices[i].SetPitchBend(val);
	}
}

//----------------------------------------------------------------------------
void bs2notemaster::Write2IPBuffer1(float inval)
{
	IPBuffer1[IPBuff1Index] = inval;

	if(IPBuff1Index < 4)
		IPBuffer1[IPBuff1Index] *= (float)IPBuff1Index/4.0f;	//so that there aren't clicks at the ends of the buffer (?)
	else if(IPBuff1Index > (BUFFERSIZE-5))
		IPBuffer1[IPBuff1Index] *= (float)(BUFFERSIZE-IPBuff1Index)/4.0f; //ditto

	IPBuff1Index++;
	if(IPBuff1Index > (BUFFERSIZE-1))
		IPBuff1Index = 0;
}

//----------------------------------------------------------------------------
void bs2notemaster::Write2IPBuffer1(float *inval)
{
	int i;

	for(i=0;i<BUFFERSIZE;i++)
		IPBuffer1[i] = inval[i];
}

//----------------------------------------------------------------------------
void bs2notemaster::Write2IPBuffer2(float inval)
{
	IPBuffer2[IPBuff2Index] = inval;

	if(IPBuff2Index < 4)
		IPBuffer2[IPBuff2Index] *= (float)IPBuff2Index/4.0f;	//so that there aren't clicks at the ends of the buffer (?)
	else if(IPBuff2Index > (BUFFERSIZE-5))
		IPBuffer2[IPBuff2Index] *= (float)(BUFFERSIZE-IPBuff2Index)/4.0f; //ditto

	IPBuff2Index++;
	if(IPBuff2Index > (BUFFERSIZE-1))
		IPBuff2Index = 0;	
}

//----------------------------------------------------------------------------
void bs2notemaster::Write2IPBuffer2(float *inval)
{
	int i;

	for(i=0;i<BUFFERSIZE;i++)
		IPBuffer2[i] = inval[i];	
}

//----------------------------------------------------------------------------
void bs2notemaster::SetParameter(long index, float val)
{
	int i;

	if(index == ksett_PolyphonicMode)
	{
		if(val < 0.5f)
		{
			if(PolyphonicMode)
			{
				for(i=1;i<NUM_VOICES;i++)
				{
					Voices[i].NoteOff();
					Notes[i] = -1;
				}
			}
			PolyphonicMode = false;
		}
		else
		{
			PolyphonicMode = true;
			Voices[0].setIsActive(true); //just in case?
		}
		return;
	}

	for(i=0;i<NUM_VOICES;i++)
		Voices[i].SetParameter(index, val);
}

//----------------------------------------------------------------------------
void bs2notemaster::SetSamplerate(float samplerate)
{
	int i;

	Samplerate = samplerate;

	for(i=0;i<NUM_VOICES;i++)
		Voices[i].SetSamplerate(Samplerate);
}

//----------------------------------------------------------------------------
void bs2notemaster::SetTempo(float tempo)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
		Voices[i].SetTempo(tempo);
}

//----------------------------------------------------------------------------
void bs2notemaster::GetBuffer1(float *buf, int size)
{
	int i;
	float j, j_inc;

	j = 0;
	j_inc = (float)BUFFERSIZE/(float)size;

	for(i=0;i<size;i++)
	{
		buf[i] = IPBuffer1[(long)j];
		j += j_inc;
	}
}

//----------------------------------------------------------------------------
void bs2notemaster::GetBuffer2(float *buf, int size)
{
	int i;
	float j, j_inc;

	j = 0;
	j_inc = (float)BUFFERSIZE/(float)size;

	for(i=0;i<size;i++)
	{
		buf[i] = IPBuffer2[(long)j];
		j += j_inc;
	}
}

//----------------------------------------------------------------------------
void bs2notemaster::GetBuffer1(float *buf)
{
	int i;

	for(i=0;i<BUFFERSIZE;i++)
		buf[i] = IPBuffer1[i];
}

//----------------------------------------------------------------------------
void bs2notemaster::GetBuffer2(float *buf)
{
	int i;

	for(i=0;i<BUFFERSIZE;i++)
		buf[i] = IPBuffer2[i];
}

//----------------------------------------------------------------------------
twofloats bs2notemaster::GetSample(bool barstart)
{
	//int i;
	twofloats retval;
	retval.left = 0.0f;
	retval.right = 0.0f;

	VIACountDown--;
	if(VIACountDown < 0)
	{
		VIACountDown = 5;
		if(Voices[0].GetIsActive())
			VoiceIsActive[0] = true;
		else
			VoiceIsActive[0] = false;
		if(Voices[1].GetIsActive())
			VoiceIsActive[1] = true;
		else
			VoiceIsActive[1] = false;
		if(Voices[2].GetIsActive())
			VoiceIsActive[2] = true;
		else
			VoiceIsActive[2] = false;
		if(Voices[3].GetIsActive())
			VoiceIsActive[3] = true;
		else
			VoiceIsActive[3] = false;
		if(Voices[4].GetIsActive())
			VoiceIsActive[4] = true;
		else
			VoiceIsActive[4] = false;
		if(Voices[5].GetIsActive())
			VoiceIsActive[5] = true;
		else
			VoiceIsActive[5] = false;
		if(Voices[6].GetIsActive())
			VoiceIsActive[6] = true;
		else
			VoiceIsActive[6] = false;
		if(Voices[7].GetIsActive())
			VoiceIsActive[7] = true;
		else
			VoiceIsActive[7] = false;
		if(Voices[8].GetIsActive())
			VoiceIsActive[8] = true;
		else
			VoiceIsActive[8] = false;
		if(Voices[9].GetIsActive())
			VoiceIsActive[9] = true;
		else
			VoiceIsActive[9] = false;
		if(Voices[10].GetIsActive())
			VoiceIsActive[10] = true;
		else
			VoiceIsActive[10] = false;
		if(Voices[11].GetIsActive())
			VoiceIsActive[11] = true;
		else
			VoiceIsActive[11] = false;
		if(Voices[12].GetIsActive())
			VoiceIsActive[12] = true;
		else
			VoiceIsActive[12] = false;
		if(Voices[13].GetIsActive())
			VoiceIsActive[13] = true;
		else
			VoiceIsActive[13] = false;
		if(Voices[14].GetIsActive())
			VoiceIsActive[14] = true;
		else
			VoiceIsActive[14] = false;
		if(Voices[15].GetIsActive())
			VoiceIsActive[15] = true;
		else
			VoiceIsActive[15] = false;
	}

	if(PolyphonicMode)
	{
		/*for(i=0;i<NUM_VOICES;i++)
		{
			if(Voices[i].GetIsActive())
				retval += Voices[i].GetSample(barstart);
		}*/ //-unroll this loop, to speed things up a bit:
		if(VoiceIsActive[0])
			retval += Voices[0].GetSample(barstart);
		if(VoiceIsActive[1])
			retval += Voices[1].GetSample(barstart);
		if(VoiceIsActive[2])
			retval += Voices[2].GetSample(barstart);
		if(VoiceIsActive[3])
			retval += Voices[3].GetSample(barstart);
		if(VoiceIsActive[4])
			retval += Voices[4].GetSample(barstart);
		if(VoiceIsActive[5])
			retval += Voices[5].GetSample(barstart);
		if(VoiceIsActive[6])
			retval += Voices[6].GetSample(barstart);
		if(VoiceIsActive[7])
			retval += Voices[7].GetSample(barstart);
		if(VoiceIsActive[8])
			retval += Voices[8].GetSample(barstart);
		if(VoiceIsActive[9])
			retval += Voices[9].GetSample(barstart);
		if(VoiceIsActive[10])
			retval += Voices[10].GetSample(barstart);
		if(VoiceIsActive[11])
			retval += Voices[11].GetSample(barstart);
		if(VoiceIsActive[12])
			retval += Voices[12].GetSample(barstart);
		if(VoiceIsActive[13])
			retval += Voices[13].GetSample(barstart);
		if(VoiceIsActive[14])
			retval += Voices[14].GetSample(barstart);
		if(VoiceIsActive[15])
			retval += Voices[15].GetSample(barstart);
	}
	else
		retval = Voices[0].GetSample(barstart);

	return retval;
}
