//	bs2voice.cpp - A single voice of Buffer Synth 2 (it slaves to a
//				   bs2notemaster object).
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
#include <stdlib.h>
#include <stdio.h>

#include "bs2voice.h"
#include "MIDI2Freq.h"
#include "bs2notemaster.h"

/*#ifndef MAC
	inline int float2int(float val)
	{
		int result;
		_asm {
			fld dword ptr [val]
			fistp result
		}
		return result;
	}
#else
	const double _double2fixmagic = 68719476736.0*1.5;
	const long _shiftamt = 16;

	#define iexp_ 0
	#define iman_ 1

	inline int float2int(float val)
	{
		double temp = (double)val;
		temp = temp + _double2fixmagic;
		return (int)(((long *)&temp)[iman_] >> _shiftamt);
	}
#endif

inline float interp(float val1, float val2, float index)
{
	float outval, index_fract;

	index_fract = index - (float)(float2int(index));
	outval = val1 + ((val2 - val1)*index_fract);

	return outval;
}*/

//Did I get this from musicdsp.org?
inline float n_tanh(float inval)
{
	//0-(1-exp(2*x))/(1+exp(2*x))
	return -(1.0f-(float)exp(2.0f*inval))/(1.0f+(float)exp(2.0f*inval));
}

bs2voice::bs2voice(bs2notemaster *notemaster, int tag, float samplerate, float *buffer1, float *buffer2, float *tanhwave, float *sinewave)
{
	NoteMaster = notemaster;

	Tag = tag;

	IsActive = false;
	ControlCount = 0;

	BaseFrequency = 440.0f;
	Frequency = BaseFrequency;
	Note = 0;
	Velocity = 1.0f;

	SampleRate = samplerate;
	InverseSampleRate = 1.0f/SampleRate;

	FirstVoice = false;

	noteOnOffFade = 0.0f;

	//----------------------------------------------------------------------------------------
	//Buffer-specific stuff
	//----------------------------------------------------------------------------------------
	Buffer1 = buffer1;
	Buffer2 = buffer2;

	Index1 = 0.0f;
	Increment1 = 1.0f;
	Index2 = 0.0f;
	Increment2 = 1.0f;
	Start1 = 0.0f * BUFFERSIZE;
	End1 = 1.0f * BUFFERSIZE;
	Start2 = 0.0f * BUFFERSIZE;
	End2 = 1.0f * BUFFERSIZE;
	Frequency1 = 440.0f;
	Frequency2 = 440.0f;

	Fades1 = 1.0f;
	Fades2 = 1.0f;

	OscillatorMode = false;
	OscMode_f = 0.0f;
	PitchCorrection = 0;

	Env = 0.0f;

	b1_RPChanged = false;
	b2_RPChanged = false;

	//----------------------------------------------------------------------------------------
	//Envelope-specific stuff
	//----------------------------------------------------------------------------------------
	TanhWaveform = tanhwave;
	NoteIsOff = true;

	Index_Env1 = 0.0f;
	CurrentSegment1 = kA;
	Index_Env2 = 0.0f;
	CurrentSegment2 = kA;

	A1 = 1.0f;
	D1 = 1.0f;
	R1 = 1.0f;
	SegmentTime1 = 1.0f;
	A2 = 1.0f;
	D2 = 1.0f;
	R2 = 1.0f;
	SegmentTime2 = 1.0f;

	Env1 = 0.0f;
	Env2 = 0.0f;

	//----------------------------------------------------------------------------------------
	//LFO-specific stuff
	//----------------------------------------------------------------------------------------
	/*LFO_Filter1 = 0;
	LFO_Filter2 = 0;
	LFO_Filter1 = new NyquistEllipFilter();
	LFO_Filter2 = new NyquistEllipFilter();*/

	SineWaveform = sinewave;
	Tempo = 120.0f;
	LFO_Frequency1 = 15.0f;
	LFO_Frequency2 = 15.0f;
	LFO_Notelength1 = nl_crotchet;
	LFO_Notelength2 = nl_crotchet;
	LFO_Waveform1 = wf_sine;
	LFO_Waveform2 = wf_sine;
	LFO_Increment1 = 1.0f;
	LFO_Increment2 = 1.0f;
	LFO_Index1 = 0.0f;
	LFO_Index2 = 0.0f;

	BarStartCountDown = 0;

	LFO1 = 0.0f;
	LFO2 = 0.0f;

	LFO_Current1 = 0.0f;
	LFO_Current2 = 0.0f;

	//----------------------------------------------------------------------------------------
	//Filter-specific stuff
	//----------------------------------------------------------------------------------------
	CutOff = 440.0f;
	Q = 0.1f;

	Low.left = 0.0f;
	Low.right = 0.0f;
	High.left = 0.0f;
	High.right = 0.0f;
	Band.left = 0.0f;
	Band.right = 0.0f;
	Notch.left = 0.0f;
	Notch.right = 0.0f;

	Type = ft_Low;

	Filter_CalcF();
}

void bs2voice::Create(bs2notemaster *notemaster, int tag, float samplerate, float *buffer1, float *buffer2, float *tanhwave, float *sinewave)
{
	NoteMaster = notemaster;

	Tag = tag;

	IsActive = false;
	ControlCount = 0;

	BaseFrequency = 440.0f;
	Frequency = BaseFrequency;
	Note = 0;
	Velocity = 1.0f;

	SampleRate = samplerate;
	InverseSampleRate = 1.0f/SampleRate;

	//----------------------------------------------------------------------------------------
	//Buffer-specific stuff
	//----------------------------------------------------------------------------------------
	Buffer1 = buffer1;
	Buffer2 = buffer2;

	Index1 = 0.0f;
	Increment1 = 1.0f;
	Index2 = 0.0f;
	Increment2 = 1.0f;
	Start1 = 0.0f * BUFFERSIZE;
	End1 = 1.0f * BUFFERSIZE;
	Start2 = 0.0f * BUFFERSIZE;
	End2 = 1.0f * BUFFERSIZE;
	Frequency1 = 440.0f;
	Frequency2 = 440.0f;

	Fades1 = 1.0f;
	Fades2 = 1.0f;

	OscillatorMode = false;
	OscMode_f = 0.0f;
	PitchCorrection = 0;

	Env = 0.0f;

	//----------------------------------------------------------------------------------------
	//Envelope-specific stuff
	//----------------------------------------------------------------------------------------
	TanhWaveform = tanhwave;
	NoteIsOff = true;

	Index_Env1 = 0.0f;
	CurrentSegment1 = kA;
	Index_Env2 = 0.0f;
	CurrentSegment2 = kA;

	A1 = 1.0f;
	D1 = 1.0f;
	R1 = 1.0f;
	SegmentTime1 = 1.0f;
	A2 = 1.0f;
	D2 = 1.0f;
	R2 = 1.0f;
	SegmentTime2 = 1.0f;

	Env1 = 0.0f;
	Env2 = 0.0f;

	e2_MIDINotesTrigger = false;
	e2_BarStartTriggers = false;
	Tempo_SegmentTime = 1.0f;

	//----------------------------------------------------------------------------------------
	//LFO-specific stuff
	//----------------------------------------------------------------------------------------
	/*LFO_Filter1 = 0;
	LFO_Filter2 = 0;
	LFO_Filter1 = new NyquistEllipFilter();
	LFO_Filter2 = new NyquistEllipFilter();*/

	SineWaveform = sinewave;
	Tempo = 120.0f;
	LFO_Frequency1 = 15.0f;
	LFO_Frequency2 = 15.0f;
	LFO_Notelength1 = nl_crotchet;
	LFO_Notelength2 = nl_crotchet;
	LFO_Waveform1 = wf_sine;
	LFO_Waveform2 = wf_sine;
	LFO_Increment1 = 1.0f;
	LFO_Increment2 = 1.0f;
	LFO_Index1 = 0.0f;
	LFO_Index2 = 0.0f;

	BarStartCountDown = 0;

	LFO1 = 0.0f;
	LFO2 = 0.0f;

	LFO_Current1 = 0.0f;
	LFO_Current2 = 0.0f;

	//----------------------------------------------------------------------------------------
	//Filter-specific stuff
	//----------------------------------------------------------------------------------------
	CutOff = 440.0f;
	Q = 0.1f;

	Low.left = 0.0f;
	Low.right = 0.0f;
	High.left = 0.0f;
	High.right = 0.0f;
	Band.left = 0.0f;
	Band.right = 0.0f;
	Notch.left = 0.0f;
	Notch.right = 0.0f;

	Type = ft_Low;

	Filter_CalcF();
}

//--------------------------------------------------------------------------------------------
void bs2voice::NoteOn(int note, float velocity)
{
	IsActive = true;

	Velocity = velocity;
	Note = note;
	BaseFrequency = MIDI2Freq[note];
	Frequency = BaseFrequency;
	NoteIsOff = false;

	if(ae_OnOff/*&&ae_MIDINotesTrigger*/)
	{
		CurrentSegment1 = kA;	//maybe update this, so there's a smooth transition?
		Index_Env1 = 0.0f;
	}
	else if(!FirstVoice)
		noteOnOffFade = 0.0f;
	if(e2_MIDINotesTrigger)
	{
		CurrentSegment2 = kA;
		Index_Env2 = 0.0f;
	}
	if(b1_ResetRPOnMIDINote)
		Index1 = Start1;
	if(b2_ResetRPOnMIDINote)
		Index1 = Start2;

	if(lfo1_MIDINotesReset)
		LFO1_ResetPhaseTo(0.0f);
	if(lfo2_MIDINotesReset)
		LFO2_ResetPhaseTo(0.0f);
}

//--------------------------------------------------------------------------------------------
void bs2voice::NoteOff()
{
	if(ae_OnOff/*&&ae_MIDINotesTrigger*/)
	{
		if(CurrentSegment1 == kA)
			ae_S_actual = Env1;
		else if(CurrentSegment1 == kD)
		{
			ae_S_actual = Env1;
			Index_Env1 = (float)TABLE_SIZE-1.0f;
		}
		CurrentSegment1 = kR;	//maybe update this, to make it smooth?
	}
	/*else if(!FirstVoice)
		IsActive = false;*/
	if(e2_MIDINotesTrigger)
	{
		if(CurrentSegment2 == kA)
			e2_S_actual = Env2;
		else if(CurrentSegment2 == kD)
		{
			e2_S_actual = Env2;
			Index_Env2 = (float)TABLE_SIZE-1.0f;
		}
		CurrentSegment2 = kR;
	}
	NoteIsOff = true;
}

//--------------------------------------------------------------------------------------------
void bs2voice::SetPitchBend(float val)
{
	float temp;
	if(val >= 0.0f)
	{
		temp = MIDI2Freq[Note+2];
		temp -= MIDI2Freq[Note];
		Frequency += temp;
	}
	else if(val < 0.0f)
	{
		temp = MIDI2Freq[Note-2];
		temp -= BaseFrequency;
		Frequency += temp;
	}
}

//--------------------------------------------------------------------------------------------
void bs2voice::SetParameter(long index, float val)
{
	FILE *fp;

	switch(index)
	{
		case ksett_SynthMode:
			if(val < 0.5f)
			{
				OscillatorMode = false;
				OscMode_f = 0.0f;
			}
			else
			{
				OscillatorMode = true;
				OscMode_f = 1.0f;
			}
			break;

		//Buffer1
		case kb1_Start:
			Start1 = val * (float)(BUFFERSIZE-1);
			//if(Start1 >= (End1-1.0f))
			if(Start1 >= End1)
				Start1 -= 2.0f;
			break;
		case kb1_End:
			End1 = val * (float)(BUFFERSIZE-1);
			//if((End1-1.0f) <= Start1)
			if(End1 <= Start1)
				End1 += 2.0f;
			break;
		case kb1_SizeFrom:
			if(val < 0.5f)
				b1_FromStart = false;
			else b1_FromStart = true;
			break;
		case kb1_Speed_Pitch:
			b1_Speed = val;
			break;
		case kb1_Level:
			b1_Level = val;
			break;
		case kb1_LinearInterp:
			if(val < 0.5f)
			{
				b1_LinearInterp = false;
				b1_LInterp_f = 0.0f;
			}
			else
			{
				b1_LinearInterp = true;
				b1_LInterp_f = 1.0f;
			}
			break;
		case kb1_Reverse:
			if(val < 0.5f)
				b1_Reverse = false;
			else
				b1_Reverse = true;
			break;
		case kb1_OnlyOPWhenFrozen:
			if(val < 0.5f)
				b1_OnlyOPWhenFrozen = false;
			else
				b1_OnlyOPWhenFrozen = true;
			break;
		case kb1_Freeze:
			if(val < 0.5f)
				b1_Freeze = false;
			else
				b1_Freeze = true;
			break;
		case kb1_ReadPosition:
			b1_ReadPosition = (val * (float)(BUFFERSIZE-1));
			b1_NewRP = (float)NoteMaster->IPBuff1Index - b1_ReadPosition - 1.0f;
			if(b1_NewRP < 0.0f)
				b1_NewRP += (BUFFERSIZE-1);
			b1_RPChanged = true;
			break;
		case kb1_ResetRPOnMIDINote:
			if(val < 0.5f)
				b1_ResetRPOnMIDINote = false;
			else
				b1_ResetRPOnMIDINote = true;
			break;
		case kb1_Pan:
			b1_Pan = val;
			break;

		//Buffer2
		case kb2_Start:
			Start2 = val * (float)BUFFERSIZE;
			//if(Start2 >= (End2-1))
			if(Start2 >= End2)
				Start2 -= 2.0f;
			break;
		case kb2_End:
			End2 = val * (float)(BUFFERSIZE-1);
			//if((End2-1) <= Start2)
			if(End2 <= Start2)
				End2 += 2.0f;
			break;
		case kb2_SizeFrom:
			if(val < 0.5f)
				b2_FromStart = false;
			else
				b2_FromStart = true;
			break;
		case kb2_Speed_Pitch:
			b2_Speed = val;
			break;
		case kb2_Level:
			b2_Level = val;
			break;
		case kb2_LinearInterp:
			if(val < 0.5f)
			{
				b2_LinearInterp = false;
				b2_LInterp_f = 0.0f;
			}
			else
			{
				b2_LinearInterp = true;
				b2_LInterp_f = 1.0f;
			}
			break;
		case kb2_Reverse:
			if(val < 0.5f)
				b2_Reverse = false;
			else
				b2_Reverse = true;
			break;
		case kb2_OnlyOPWhenFrozen:
			if(val < 0.5f)
				b2_OnlyOPWhenFrozen = false;
			else
				b2_OnlyOPWhenFrozen = true;
			break;
		case kb2_Freeze:
			if(val < 0.5f)
				b2_Freeze = false;
			else
				b2_Freeze = true;
			break;
		case kb2_ModDepth:
			b2_ModDepth = val;
			break;
		case kb2_ModDestination:
			if(val < (1.0f/10.0f))
				b2_Destination = dest_off;
			else if(val < (2.0f/10.0f))
				b2_Destination = dest_buf1level;
			else if(val < (3.0f/10.0f))
				b2_Destination = dest_buf1speed;
			else if(val < (4.0f/10.0f))
				b2_Destination = dest_buf1size;
			else if(val < (5.0f/10.0f))
				b2_Destination = dest_buf1start;
			else if(val < (6.0f/10.0f))
				b2_Destination = dest_buf1end;
			else if(val < (7.0f/10.0f))
				b2_Destination = dest_filtcutoff;
			else if(val < (8.0f/10.0f))
				b2_Destination = dest_filtresonance;
			else if(val < (9.0f/10.0f))
				b2_Destination = dest_buf1pan;
			else
				b2_Destination = dest_buf1readp;
			break;
		case kb2_Envelope:
			if(val < 0.5f)
				b2_Envelope = false;
			else
				b2_Envelope = true;
			break;
		case kb2_ReadPosition:
			b2_ReadPosition = (val * (float)(BUFFERSIZE-1));
			b2_NewRP = (float)NoteMaster->IPBuff2Index - b2_ReadPosition - 1.0f;
			if(b2_NewRP < 0.0f)
				b2_NewRP += (BUFFERSIZE-1);
			b2_RPChanged = true;
			break;
		case kb2_ResetRPOnMIDINote:
			if(val < 0.5f)
				b2_ResetRPOnMIDINote = false;
			else
				b2_ResetRPOnMIDINote = true;
			break;
		case kb2_Pan:
			b2_Pan = val;
			break;

		//Amplitude Envelope
		case kae_OnOff:
			if(val < 0.5f)
				ae_OnOff = false;
			else
			{
				ae_OnOff = true;
				NoteOff();
			}
			break;
		case kae_Attack:
			A1 = val;
			IncrementA1 = A1 * SegmentTime1;
			if(IncrementA1 == 0.0f)
				IncrementA1 += 0.0001f;
			IncrementA1 = (1.0f/IncrementA1) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		case kae_Decay:
			D1 = val;
			IncrementD1 = D1 * SegmentTime1;
			if(IncrementD1 == 0.0f)
				IncrementD1 += 0.0001f;
			IncrementD1 = (1.0f/IncrementD1) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		case kae_Release:
			R1 = val;
			IncrementR1 = R1 * SegmentTime1;
			if(IncrementR1 == 0.0f)
				IncrementR1 += 0.0001f;
			IncrementR1 = (1.0f/IncrementR1) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		case kae_Sustain:
			ae_S = val;
			ae_S_actual = ae_S;
			break;
		case kae_SegmentTime:
			SegmentTime1 = val * 5.0f;
			IncrementA1 = A1 * SegmentTime1;
			IncrementD1 = D1 * SegmentTime1;
			IncrementR1 = R1 * SegmentTime1;
			IncrementA1 = (1.0f/IncrementA1) * ((float)TABLE_SIZE*InverseSampleRate);
			IncrementD1 = (1.0f/IncrementD1) * ((float)TABLE_SIZE*InverseSampleRate);
			IncrementR1 = (1.0f/IncrementR1) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		/*case kae_MIDINotesTrigger:
			if(val < 0.5f)
				ae_MIDINotesTrigger = false;
			else
				ae_MIDINotesTrigger = true;
			break;*/

		//2nd Envelope
		case ke2_Attack:
			A2 = val;
			if(!e2_BarStartTriggers)
				IncrementA2 = A2 * SegmentTime2;
			else
			{
				IncrementA2 = A2 * Tempo_SegmentTime;

				IncrementS2 = (Tempo_SegmentTime * 4.0f) - (IncrementA2 + (D2*Tempo_SegmentTime) + (IncrementR2*Tempo_SegmentTime));
				IncrementS2 = (1.0f/IncrementS2) * ((float)TABLE_SIZE*InverseSampleRate);
			}
			if(IncrementA2 == 0.0f)
				IncrementA2 += 0.0001f;
			IncrementA2 = (1.0f/IncrementA2) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		case ke2_Decay:
			D2 = val;
			if(!e2_BarStartTriggers)
				IncrementD2 = D2 * SegmentTime2;
			else
			{
				IncrementD2 = D2 * Tempo_SegmentTime;

				IncrementS2 = (Tempo_SegmentTime * 4.0f) - ((IncrementA2*Tempo_SegmentTime) + IncrementD2 + (IncrementR2*Tempo_SegmentTime));
				IncrementS2 = (1.0f/IncrementS2) * ((float)TABLE_SIZE*InverseSampleRate);
			}
			if(IncrementD2 == 0.0f)
				IncrementD2 += 0.0001f;
			IncrementD2 = (1.0f/IncrementD2) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		case ke2_Release:
			R2 = val;
			if(!e2_BarStartTriggers)
				IncrementR2 = R2 * SegmentTime2;
			else
			{
				IncrementR2 = R2 * Tempo_SegmentTime;

				IncrementS2 = (Tempo_SegmentTime * 4.0f) - ((IncrementA2*Tempo_SegmentTime) + (IncrementD2*Tempo_SegmentTime) + IncrementR2);
				IncrementS2 = (1.0f/IncrementS2) * ((float)TABLE_SIZE*InverseSampleRate);
			}
			if(IncrementR2 == 0.0f)
				IncrementR2 += 0.0001f;
			IncrementR2 = (1.0f/IncrementR2) * ((float)TABLE_SIZE*InverseSampleRate);
			break;
		case ke2_Sustain:
			e2_S = val;
			e2_S_actual = e2_S;
			break;
		case ke2_SegmentTime:
			SegmentTime2 = val * 5.0f;
			if(!e2_BarStartTriggers)
			{
				IncrementA2 = A2 * SegmentTime2;
				IncrementD2 = D2 * SegmentTime2;
				IncrementR2 = R2 * SegmentTime2;
				IncrementA2 = (1.0f/IncrementA2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementD2 = (1.0f/IncrementD2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementR2 = (1.0f/IncrementR2) * ((float)TABLE_SIZE*InverseSampleRate);
			}
			break;
		case ke2_MIDINotesTrigger:
			if(val < 0.5f)
				e2_MIDINotesTrigger = false;
			else
				e2_MIDINotesTrigger = true;
			break;
		case ke2_BarStartTriggers:
			if(val < 0.5f)
			{
				e2_BarStartTriggers = false;

				IncrementA2 = A2 * SegmentTime2;
				IncrementD2 = D2 * SegmentTime2;
				IncrementR2 = R2 * SegmentTime2;
				IncrementA2 = (1.0f/IncrementA2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementD2 = (1.0f/IncrementD2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementR2 = (1.0f/IncrementR2) * ((float)TABLE_SIZE*InverseSampleRate);
			}
			else
			{
				e2_BarStartTriggers = true;

				IncrementA2 = A2 * Tempo_SegmentTime;
				IncrementD2 = D2 * Tempo_SegmentTime;
				IncrementR2 = R2 * Tempo_SegmentTime;
				IncrementS2 = (Tempo_SegmentTime * 4.0f) - (IncrementA2 + IncrementD2 + IncrementR2);
				IncrementA2 = (1.0f/IncrementA2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementD2 = (1.0f/IncrementD2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementS2 = (1.0f/IncrementS2) * ((float)TABLE_SIZE*InverseSampleRate);
				IncrementR2 = (1.0f/IncrementR2) * ((float)TABLE_SIZE*InverseSampleRate);

				CurrentSegment2 = kA;
				Index_Env2 = 0.0f;
			}
			break;
		case ke2_Direction:
			if(val < 0.5f)
				e2_InvertEnvelope = false;
			else
				e2_InvertEnvelope = true;
			break;
		case ke2_ModDepth:
			e2_ModDepth = val;
			break;
		case ke2_Destination:
			if(val < (1.0f/22.0f))
				e2_Destination = dest_off;
			else if(val < (2.0f/22.0f))
				e2_Destination = dest_buf1level;
			else if(val < (3.0f/22.0f))
				e2_Destination = dest_buf1speed;
			else if(val < (4.0f/22.0f))
				e2_Destination = dest_buf1size;
			else if(val < (5.0f/22.0f))
				e2_Destination = dest_buf1start;
			else if(val < (6.0f/22.0f))
				e2_Destination = dest_buf1end;
			else if(val < (7.0f/22.0f))
				e2_Destination = dest_buf2level;
			else if(val < (8.0f/22.0f))
				e2_Destination = dest_buf2speed;
			else if(val < (9.0f/22.0f))
				e2_Destination = dest_buf2size;
			else if(val < (10.0f/22.0f))
				e2_Destination = dest_buf2start;
			else if(val < (11.0f/22.0f))
				e2_Destination = dest_buf2end;
			else if(val < (12.0f/22.0f))
				e2_Destination = dest_filtcutoff;
			else if(val < (13.0f/22.0f))
				e2_Destination = dest_filtresonance;
			else if(val < (14.0f/22.0f))
				e2_Destination = dest_lfo1depth;
			else if(val < (15.0f/22.0f))
				e2_Destination = dest_lfo2depth;
			else if(val < (16.0f/22.0f))
				e2_Destination = dest_buf1pan;
			else if(val < (17.0f/22.0f))
				e2_Destination = dest_buf1readp;
			else if(val < (18.0f/22.0f))
				e2_Destination = dest_buf2pan;
			else if(val < (19.0f/22.0f))
				e2_Destination = dest_buf2readp;
			else if(val < (20.0f/22.0f))
				e2_Destination = dest_buf2depth;
			else if(val < (21.0f/22.0f))
				e2_Destination = dest_lfo1nfreq;
			else
				e2_Destination = dest_lfo2nfreq;
			break;

		//LFO1
		case klfo1_Freq_Note:
			if(!lfo1_TempoSync)
			{
				LFO_Frequency1 = val * LFO_MAX_FREQ;
				LFO_Increment1 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment1 *= LFO_Frequency1;
			}
			else
			{
				LFO_Notelength1 = Float2Note(val);
				LFO_Increment1 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment1 *= CalcFreq(LFO_Notelength1);
			}
			break;
		case klfo1_TempoSync:
			if(val < 0.5f)
			{
				lfo1_TempoSync = false;
				LFO_Frequency1 = val * LFO_MAX_FREQ;
				LFO_Increment1 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment1 *= LFO_Frequency1;
			}
			else
			{
				lfo1_TempoSync = true;
				LFO_Notelength1 = Float2Note(val);
				LFO_Increment1 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment1 *= CalcFreq(LFO_Notelength1);
			}
			break;
		case klfo1_Waveform:
			LFO_Waveform1 = Float2Wform(val);
			break;
		case klfo1_BarStartResets:
			if(val < 0.5f)
				lfo1_BarStartResets = false;
			else
				lfo1_BarStartResets = true;
			break;
		case klfo1_MIDINotesReset:
			if(val < 0.5f)
				lfo1_MIDINotesReset = false;
			else
				lfo1_MIDINotesReset = true;
			break;
		case klfo1_Destination:
			if(val < (1.0f/18.0f))
				lfo1_Destination = dest_off;
			else if(val < (2.0f/18.0f))
				lfo1_Destination = dest_buf1level;
			else if(val < (3.0f/18.0f))
				lfo1_Destination = dest_buf1speed;
			else if(val < (4.0f/18.0f))
				lfo1_Destination = dest_buf1size;
			else if(val < (5.0f/18.0f))
				lfo1_Destination = dest_buf1start;
			else if(val < (6.0f/18.0f))
				lfo1_Destination = dest_buf1end;
			else if(val < (7.0f/18.0f))
				lfo1_Destination = dest_buf2level;
			else if(val < (8.0f/18.0f))
				lfo1_Destination = dest_buf2speed;
			else if(val < (9.0f/18.0f))
				lfo1_Destination = dest_buf2size;
			else if(val < (10.0f/18.0f))
				lfo1_Destination = dest_buf2start;
			else if(val < (11.0f/18.0f))
				lfo1_Destination = dest_buf2end;
			else if(val < (12.0f/18.0f))
				lfo1_Destination = dest_filtcutoff;
			else if(val < (13.0f/18.0f))
				lfo1_Destination = dest_filtresonance;
			else if(val < (14.0f/18.0f))
				lfo1_Destination = dest_buf1pan;
			else if(val < (15.0f/18.0f))
				lfo1_Destination = dest_buf1readp;
			else if(val < (16.0f/18.0f))
				lfo1_Destination = dest_buf2pan;
			else if(val < (17.0f/18.0f))
				lfo1_Destination = dest_buf2readp;
			else
				lfo1_Destination = dest_buf2depth;
			break;
		case klfo1_ModDepth:
			lfo1_ModDepth = val;
			break;

		//LFO2
		case klfo2_Freq_Note:
			if(!lfo2_TempoSync)
			{
				LFO_Frequency2 = val * LFO_MAX_FREQ;
				LFO_Increment2 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment2 *= LFO_Frequency2;
			}
			else
			{
				LFO_Notelength2 = Float2Note(val);
				LFO_Increment2 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment2 *= CalcFreq(LFO_Notelength2);
			}
			break;
		case klfo2_TempoSync:
			if(val < 0.5f)
			{
				lfo2_TempoSync = false;
				LFO_Frequency2 = val * LFO_MAX_FREQ;
				LFO_Increment2 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment2 *= LFO_Frequency2;
			}
			else
			{
				lfo2_TempoSync = true;
				LFO_Notelength2 = Float2Note(val);
				LFO_Increment2 = TABLE_SIZE*InverseSampleRate;
				LFO_Increment2 *= CalcFreq(LFO_Notelength2);
			}
			break;
		case klfo2_Waveform:
			LFO_Waveform2 = Float2Wform(val);
			break;
		case klfo2_BarStartResets:
			if(val < 0.5f)
				lfo2_BarStartResets = false;
			else
				lfo2_BarStartResets = true;
			break;
		case klfo2_MIDINotesReset:
			if(val < 0.5f)
				lfo2_MIDINotesReset = false;
			else
				lfo2_MIDINotesReset = true;
			break;
		case klfo2_Destination:
			if(val < (1.0f/18.0f))
				lfo2_Destination = dest_off;
			else if(val < (2.0f/18.0f))
				lfo2_Destination = dest_buf1level;
			else if(val < (3.0f/18.0f))
				lfo2_Destination = dest_buf1speed;
			else if(val < (4.0f/18.0f))
				lfo2_Destination = dest_buf1size;
			else if(val < (5.0f/18.0f))
				lfo2_Destination = dest_buf1start;
			else if(val < (6.0f/18.0f))
				lfo2_Destination = dest_buf1end;
			else if(val < (7.0f/18.0f))
				lfo2_Destination = dest_buf2level;
			else if(val < (8.0f/18.0f))
				lfo2_Destination = dest_buf2speed;
			else if(val < (9.0f/18.0f))
				lfo2_Destination = dest_buf2size;
			else if(val < (10.0f/18.0f))
				lfo2_Destination = dest_buf2start;
			else if(val < (11.0f/18.0f))
				lfo2_Destination = dest_buf2end;
			else if(val < (12.0f/18.0f))
				lfo2_Destination = dest_filtcutoff;
			else if(val < (13.0f/18.0f))
				lfo2_Destination = dest_filtresonance;
			else if(val < (14.0f/18.0f))
				lfo2_Destination = dest_buf1pan;
			else if(val < (15.0f/18.0f))
				lfo2_Destination = dest_buf1readp;
			else if(val < (16.0f/18.0f))
				lfo2_Destination = dest_buf2pan;
			else if(val < (17.0f/18.0f))
				lfo2_Destination = dest_buf2readp;
			else
				lfo2_Destination = dest_buf2depth;
			break;
		case klfo2_ModDepth:
			lfo2_ModDepth = val;
			break;

		//Filter
		case kfilt_OnOff:
			if(val < 0.5f)
				FiltOn = false;
			else
				FiltOn = true;
			break;
		case kfilt_Cutoff:
			if(val > 0.0f)
				CutOff = FILTER_MAX * val;
			else
				CutOff = 0.5f;
			Filter_CalcF();
			break;
		case kfilt_Resonance:
			Q = 1.0f - val;
			break;
		case kfilt_Type:
			if(val < (1.0f/3.0f))
				Type = ft_High;
			else if(val < (2.0f/3.0f))
				Type = ft_Band;
			else
				Type = ft_Low;
			break;

		/*case ksett_EnsureCorrectPitch_Brutal:
			if(val < 0.5f)
				EnsureCorrectPitch_Brutal = false;
			else
				EnsureCorrectPitch_Brutal = true;
			break;
		case ksett_EnsureCorrectPitch_Nice:
			if(val < 0.5f)
				EnsureCorrectPitch_Nice = false;
			else
				EnsureCorrectPitch_Nice = true;
			break;*/
		case ksett_PitchCorrection:
			if(val < 0.33f)
				PitchCorrection = 0;
			else if(val < 0.66f)
				PitchCorrection = 1;
			else
				PitchCorrection = 2;
			break;
	}
}

//--------------------------------------------------------------------------------------------
//this should really be in BufferSynth2Editor, so it can update the GUI
//--------------------------------------------------------------------------------------------
/*void bs2voice::SetSize2Tempo(note noteval, int buffer)
{
	float tempfreq;

	switch(noteval)
	{
		case nl_breve:
			tempfreq = CalcFreq(noteval);

	}
}*/

//--------------------------------------------------------------------------------------------
void bs2voice::SetTempo(float tempo)
{
	Tempo = tempo;

	Tempo_SegmentTime = 60.0f/Tempo;

	if(lfo1_TempoSync)
	{
		
		//LFO_Notelength1 = Float2Note(val);
		LFO_Increment1 = TABLE_SIZE*InverseSampleRate;
		LFO_Increment1 *= CalcFreq(LFO_Notelength1);
	}

	if(lfo2_TempoSync)
	{
		
		//LFO_Notelength2 = Float2Note();
		LFO_Increment2 = TABLE_SIZE*InverseSampleRate;
		LFO_Increment2 *= CalcFreq(LFO_Notelength2);
	}

	if(e2_BarStartTriggers)
	{
		IncrementA2 = A2 * Tempo_SegmentTime;
		IncrementD2 = D2 * Tempo_SegmentTime;
		IncrementR2 = R2 * Tempo_SegmentTime;
		IncrementS2 = (Tempo_SegmentTime * 4.0f) - (IncrementA2 + IncrementD2 + IncrementR2);
		IncrementA2 = (1.0f/IncrementA2) * ((float)TABLE_SIZE*InverseSampleRate);
		IncrementD2 = (1.0f/IncrementD2) * ((float)TABLE_SIZE*InverseSampleRate);
		IncrementS2 = (1.0f/IncrementS2) * ((float)TABLE_SIZE*InverseSampleRate);
		IncrementR2 = (1.0f/IncrementR2) * ((float)TABLE_SIZE*InverseSampleRate);
	}
}

//--------------------------------------------------------------------------------------------
twofloats bs2voice::GetSample(bool barstart)
{
	int tempIndex;
	float temp1, temp2;
	unsigned long val1, val2;
	twofloats retval;
	retval.left = 0.0f;
	retval.right = 0.0f;
	float ampval = 0.0f;

	/*if((!FirstVoice)&&NoteIsOff)
	{
		//Hmm... not sure if you'll ever get here?
		retval.left = 0.0f;
		retval.right = 0.0f;
		return retval;
	}*/

	if(b1_RPChanged||b2_RPChanged)
	{
		Index1 = (float)(NoteMaster->IPBuff1Index-1) - b1_ReadPosition;
		//Index1 = (float)(NoteMaster->IPBuff1Index-1) - 0.0f;
		if(Index1 < 0.0f)
			Index1 += (BUFFERSIZE-1);
		b1_RPChanged = false;

		Index2 = (float)(NoteMaster->IPBuff2Index-1) - b2_ReadPosition;
		//Index2 = (float)(NoteMaster->IPBuff2Index-1) - 0.0f;
		if(Index2 < 0.0f)
			Index2 += (BUFFERSIZE-1);
		b2_RPChanged = false;
	}
	/*if(b2_RPChanged)
	{
		Index2 = (float)NoteMaster->IPBuff2Index - b2_ReadPosition;
		if(Index2 < 0.0f)
			Index2 += (BUFFERSIZE-1);
		b2_RPChanged = false;
	}*/

	float b1levelval,
		  b1speedval,
		  b1sizeval,
		  b1startval,
		  b1endval,
		  b2levelval,
		  b2speedval,
		  b2sizeval,
		  b2startval,
		  b2endval,
		  filtcutoffval,
		  filtresval,
		  lfo1depthval,
		  lfo2depthval,
		  b1readpval,
		  b2readpval,
		  b1panval,
		  b2panval;

	float Env1temp;
	float Env2temp;
	float LFO1temp;
	float LFO2temp;
	float Buff2l = 0.0f;
	float Buff2r = 0.0f;
	float Buff1l = 0.0f;
	float Buff1r = 0.0f;
	float Inc1temp = 1.0f;
	float Inc2temp = 1.0f;
	float Buff2temp = 0.0f;

	//long b2_gaptemp;

	float Cond1temp1, Cond1temp2;	//used as conditionals, to avoid branching (e.g. if(bool) =>
	float Cond2temp1, Cond2temp2;	//Cond1temp1=blah*bool_float, Cond1temp2=blah*(1-bool_float)

	b1levelval = b1_Level;
	b1speedval = b1_Speed;
	b1startval = Start1;
	b1endval = End1;
	b1sizeval = End1 - Start1;
	b2levelval = b2_Level;
	b2speedval = b2_Speed;
	b2startval = Start2;
	b2endval = End2;
	b2sizeval = End2 - Start2;
	filtcutoffval = CutOff;
	filtresval = Q;
	lfo1depthval = lfo1_ModDepth;
	lfo2depthval = lfo2_ModDepth;
	b1readpval = b1_ReadPosition;
	b2readpval = b2_ReadPosition;
	b1panval = b1_Pan;
	b2panval = b2_Pan;

	if(BarStartCountDown > 0)
		BarStartCountDown--;
	if(barstart)	//because control signals are only calculated every 5 samples, need to have a way of informing the lfos when there's been a bar start
		BarStartCountDown = 6;

	LFO_Index1 += LFO_Increment1;
	LFO_Index2 += LFO_Increment2;

	//short fade in/out when amplitude envelope's not on
	if((!FirstVoice)&&(!ae_OnOff))
	{
		if((!NoteIsOff)&&(noteOnOffFade < 1.0f))
			noteOnOffFade += (1.0f/32.0f);
		else if((NoteIsOff)&&(noteOnOffFade > 0.0f))
			noteOnOffFade -= (1.0f/32.0f);
		else if(NoteIsOff)
			IsActive = false;
	}

	ControlCount++;
	if(ControlCount > 5)
	{

		//------------------------------------------------------------------------------------
		//Envelope-specific stuff
		//------------------------------------------------------------------------------------
		if(ae_OnOff)			//calculate the current value of the amplitude envelope
		{
			tempIndex = float2int(Index_Env1);
			switch(CurrentSegment1)
			{
				case kA:
					if(!NoteIsOff)
					{
						Env1 = interp(TanhWaveform[tempIndex], TanhWaveform[tempIndex+1], Index_Env1);
						Env1 += 1.0f;
						Env1 *= 0.5f;
						Index_Env1 += IncrementA1;	//because it's only called every 5 samples
						Index_Env1 += IncrementA1;
						Index_Env1 += IncrementA1;
						Index_Env1 += IncrementA1;
						Index_Env1 += IncrementA1;
						if(Index_Env1 > (float)(TABLE_SIZE-1))
						{
							CurrentSegment1 = kD;
							Index_Env1 = (float)TABLE_SIZE-1.0f;
						}
					}
					break;
				case kD:
					if(!NoteIsOff)
					{
						Env1 = interp(TanhWaveform[tempIndex], TanhWaveform[tempIndex-1], Index_Env1);
						Env1 += 1.0f;
						Env1 *= 0.5f;
						Env1 *= (1.0f-ae_S);
						Env1 += ae_S;
						Index_Env1 -= IncrementD1;
						Index_Env1 -= IncrementD1;
						Index_Env1 -= IncrementD1;
						Index_Env1 -= IncrementD1;
						Index_Env1 -= IncrementD1;
						if(Index_Env1 < 1.0f)
						{
							CurrentSegment1 = kS;
							Index_Env1 = (float)TABLE_SIZE-1.0f;
						}
					}
					break;
				case kS:
					Env1 = ae_S;
					if(NoteIsOff)
					{
						CurrentSegment1 = kR;
						Index_Env1 = (float)TABLE_SIZE-1.0f;
					}
					break;
				case kR:
					Env1 = interp(TanhWaveform[tempIndex], TanhWaveform[tempIndex-1], Index_Env1);
					Env1 += 1.0f;
					Env1 *= 0.5f;
					Env1 *= ae_S_actual;
					Index_Env1 -= IncrementR1;
					Index_Env1 -= IncrementR1;
					Index_Env1 -= IncrementR1;
					Index_Env1 -= IncrementR1;
					Index_Env1 -= IncrementR1;
					if(Index_Env1 < 1.0f)
					{
						CurrentSegment1 = kA;
						Index_Env1 = 0;
						//NoteMaster->VoiceOff(int Tag);	//****MUST REMEMBER TO UN-COMMENT THIS?****
						IsActive = false;
						ae_S_actual = ae_S;
					}
					break;
			}
		}
		else
			Env1 = 1.0f;

		if(e2_Destination != dest_off)	//calculate the current value of the second envelope
		{
			if(e2_BarStartTriggers)
			{
				if(BarStartCountDown > 0)
				{
					CurrentSegment2 = kA;
					Index_Env2 = 0.0f;
				}
			}
			tempIndex = float2int(Index_Env2);
			switch(CurrentSegment2)
			{
				case kA:
					if((!NoteIsOff)||(e2_BarStartTriggers))
					{
						Env2 = interp(TanhWaveform[tempIndex], TanhWaveform[tempIndex+1], Index_Env2);
						Env2 += 1.0f;
						Env2 *= 0.5f;
						Index_Env2 += IncrementA2;
						Index_Env2 += IncrementA2;
						Index_Env2 += IncrementA2;
						Index_Env2 += IncrementA2;
						Index_Env2 += IncrementA2;
						if(Index_Env2 > (float)(TABLE_SIZE-1))
						{
							CurrentSegment2 = kD;
							Index_Env2 = (float)TABLE_SIZE-1.0f;
						}
					}
					break;
				case kD:
					if((!NoteIsOff)||(e2_BarStartTriggers))
					{
						Env2 = interp(TanhWaveform[tempIndex], TanhWaveform[tempIndex-1], Index_Env2);
						Env2 += 1.0f;
						Env2 *= 0.5f;
						Env2 *= (1.0f-e2_S);
						Env2 += e2_S;
						Index_Env2 -= IncrementD2;
						Index_Env2 -= IncrementD2;
						Index_Env2 -= IncrementD2;
						Index_Env2 -= IncrementD2;
						Index_Env2 -= IncrementD2;
						if(Index_Env2 < 1.0f)
						{
							CurrentSegment2 = kS;
							Index_Env2 = (float)TABLE_SIZE-1.0f;
						}
					}
					break;
				case kS:
					Env2 = e2_S;
					if(e2_BarStartTriggers)
					{
						Index_Env2 -= IncrementS2;
						Index_Env2 -= IncrementS2;
						Index_Env2 -= IncrementS2;
						Index_Env2 -= IncrementS2;
						Index_Env2 -= IncrementS2;
						if(Index_Env2 < 1.0f)
							NoteIsOff = true;
					}
					if(NoteIsOff)
					{
						CurrentSegment2 = kR;
						Index_Env2 = (float)TABLE_SIZE-1.0f;
					}
					break;
				case kR:
					Env2 = interp(TanhWaveform[tempIndex], TanhWaveform[tempIndex-1], Index_Env2);
					Env2 += 1.0f;
					Env2 *= 0.5f;
					Env2 *= e2_S_actual;
					Index_Env2 -= IncrementR2;
					Index_Env2 -= IncrementR2;
					Index_Env2 -= IncrementR2;
					Index_Env2 -= IncrementR2;
					Index_Env2 -= IncrementR2;
					if(Index_Env2 < 1.0f)
					{
						CurrentSegment2 = kA;
						Index_Env2 = 0;
						//NoteMaster->VoiceOff(int Tag);	//****MUST REMEMBER TO UN-COMMENT THIS????****
						//IsActive = false;
						e2_S_actual = e2_S;
					}
					break;
			}
		}
		//------------------------------------------------------------------------------------
		//LFO-specific stuff
		//------------------------------------------------------------------------------------
		if(lfo1_Destination != dest_off)	//calculate the current value of the first lfo
		{
			if((BarStartCountDown > 0)&&(lfo1_BarStartResets))
			{
				LFO1_ResetPhaseTo(0.0f);
			}

			switch(LFO_Waveform1)
			{
				case wf_sine:
					tempIndex = float2int(LFO_Index1);
					if(tempIndex != (TABLE_SIZE-1))
						LFO1 = interp(SineWaveform[tempIndex], SineWaveform[tempIndex+1], LFO_Index1);
					else
						LFO1 = interp(SineWaveform[tempIndex], SineWaveform[0], LFO_Index1);
					break;
				case wf_saw:
					temp1 = LFO_Index1 - ((float)TABLE_SIZE * 0.5f);
					temp1 *= (INV_TS * 2.0f);	//gets a ramp waveform (/)
					temp1 *= -1.0f;				//turns it into a sawtooth (\)

					if(LFO_Index1 < 16.0f)
						temp1 *= (LFO_Index1/16.0f);
					else if(LFO_Index1 > (2048.0f-16.0f))
						temp1 *= 1.0f - ((LFO_Index1-(2048.0f-16.0f))/16.0f);

					LFO1 = temp1;
					//LFO1 = LFO_Filter1->GetSample(LFO1);
					break;
				case wf_squ:
					if(LFO_Index1 < 1024.0f)
					{
						if(LFO_Index1 < 16.0f)
							LFO1 = (LFO_Index1/16.0f);
						else if(LFO_Index1 > (1024.0f-16.0f))
							LFO1 = 1.0f - ((LFO_Index1-(1024.0f-16.0f))/16.0f);
						else
							LFO1 = 1.0f;
					}
					else
					{
						if(LFO_Index1 < (1024.0f+16.0f))
							LFO1 = -((LFO_Index1-1024.0f)/16.0f);
						else if(LFO_Index1 > (2048.0f-16.0f))
							LFO1 = -(1.0f - ((LFO_Index1-(2048.0f-16.0f))/16.0f));
						else
							LFO1 = -1.0f;
					}
					//LFO1 = LFO_Filter1->GetSample(LFO1);
					break;
				case wf_sh:
					if(LFO_Index1 < 1024.0f)
						LFO_CurrentVal1 = true;
					else
						LFO_CurrentVal1 = false;
					if(LFO_OldVal1 != LFO_CurrentVal1)
					{
						LFO_Current1 = (float)rand() - ((float)RAND_MAX * 0.5f);
						LFO_Current1 /= ((float)RAND_MAX*0.5f);
						//LFO1 = LFO_Current1;
						LFO_OldVal1 = LFO_CurrentVal1;
					}
					LFO1 = LFO_Current1;
					//LFO1 = LFO_Filter1->GetSample(LFO1);
					break;
				case wf_ramp:
					temp1 = LFO_Index1 - ((float)TABLE_SIZE * 0.5f);
					temp1 *= (INV_TS * 2.0f);	//gets a ramp waveform (/)

					if(LFO_Index1 < 16.0f)
						temp1 *= (LFO_Index1/16.0f);
					else if(LFO_Index1 > (2048.0f-16.0f))
						temp1 *= 1.0f - ((LFO_Index1-(2048.0f-16.0f))/16.0f);

					LFO1 = temp1;
					//LFO1 = LFO_Filter1->GetSample(LFO1);
					break;
			}

			//LFO_Index1 += LFO_Increment1;	//all the waveforms have the same table size
			if(LFO_Index1 > (float)(TABLE_SIZE-1))
			{
				temp1 = LFO_Index1;
				temp2 = (float)(temp1-float2int(temp1));
				temp1 = (float)(float2int(temp1) % TABLE_SIZE_INT);
				temp1 += temp2;
				LFO_Index1 = temp1;
			}
		}
		if(lfo2_Destination != dest_off)	//calculate the current value of the second lfo
		{
			if((BarStartCountDown > 0)&&(lfo2_BarStartResets))
			{
				LFO2_ResetPhaseTo(0.0f);
			}

			switch(LFO_Waveform2)
			{
				case wf_sine:
					tempIndex = float2int(LFO_Index2);
					LFO2 = interp(SineWaveform[tempIndex], SineWaveform[tempIndex+1], LFO_Index2);
					break;
				case wf_saw:
					temp1 = LFO_Index2 - ((float)TABLE_SIZE * 0.5f);
					temp1 *= (INV_TS * 2.0f);	//gets a ramp waveform (/)
					temp1 *= -1.0f;				//turns it into a sawtooth (\)

					if(LFO_Index2 < 16.0f)
						temp1 *= (LFO_Index2/16.0f);
					else if(LFO_Index2 > (2048.0f-16.0f))
						temp1 *= 1.0f - ((LFO_Index2-(2048.0f-16.0f))/16.0f);

					LFO2 = temp1;
					//LFO2 = LFO_Filter2->GetSample(LFO2);
					break;
				case wf_squ:
					if(LFO_Index2 < 1024.0f)
					{
						if(LFO_Index2 < 16.0f)
							LFO2 = (LFO_Index2/16.0f);
						else if(LFO_Index2 > (1024.0f-16.0f))
							LFO2 = 1.0f - ((LFO_Index2-(1024.0f-16.0f))/16.0f);
						else
							LFO2 = 1.0f;
					}
					else
					{
						if(LFO_Index2 < (1024.0f+16.0f))
							LFO2 = -((LFO_Index2-1024.0f)/16.0f);
						else if(LFO_Index2 > (2048.0f-16.0f))
							LFO2 = -(1.0f - ((LFO_Index2-(2048.0f-16.0f))/16.0f));
						else
							LFO2 = -1.0f;
					}
					//LFO2 = LFO_Filter2->GetSample(LFO2);
					break;
				case wf_sh:
					if(LFO_Index2 < 1024.0f)
						LFO_CurrentVal2 = true;
					else
						LFO_CurrentVal2 = false;
					if(LFO_OldVal2 != LFO_CurrentVal2)
					{
						LFO_Current2 = (float)rand() - ((float)RAND_MAX * 0.5f);
						LFO_Current2 /= ((float)RAND_MAX*0.5f);
						//LFO2 = LFO_Current2;
						LFO_OldVal2 = LFO_CurrentVal2;
					}
					LFO2 = LFO_Current2;
					//LFO2 = LFO_Filter2->GetSample(LFO2);
					break;
				case wf_ramp:
					temp1 = LFO_Index2 - ((float)TABLE_SIZE * 0.5f);
					temp1 *= (INV_TS * 2.0f);	//gets a ramp waveform (/)

					if(LFO_Index2 < 16.0f)
						temp1 *= (LFO_Index2/16.0f);
					else if(LFO_Index2 > (2048.0f-16.0f))
						temp1 *= 1.0f - ((LFO_Index2-(2048.0f-16.0f))/16.0f);

					LFO2 = temp1;
					//LFO2 = LFO_Filter2->GetSample(LFO2);
					break;
			}

			//LFO_Index2 += LFO_Increment2;	//all the waveforms have the same table size
			if(LFO_Index2 > (float)(TABLE_SIZE-1))
			{
				temp1 = LFO_Index2;
				temp2 = (float)(temp1-float2int(temp1));
				temp1 = (float)(float2int(temp1) % TABLE_SIZE_INT);
				temp1 += temp2;
				LFO_Index2 = temp1;
			}
		}
		ControlCount = 0;
	}
	else
	{
		if(LFO_Index1 > (float)(TABLE_SIZE-1))
		{
			temp1 = LFO_Index1;
			temp2 = (float)(temp1-float2int(temp1));
			temp1 = (float)(float2int(temp1) % TABLE_SIZE_INT);
			temp1 += temp2;
			LFO_Index1 = temp1;
		}

		if(LFO_Index2 > (float)(TABLE_SIZE-1))
		{
			temp1 = LFO_Index2;
			temp2 = (float)(temp1-float2int(temp1));
			temp1 = (float)(float2int(temp1) % TABLE_SIZE_INT);
			temp1 += temp2;
			LFO_Index2 = temp1;
		}
	}

	b1levelval *= Env1;
	b2levelval *= Env1;
	if((!FirstVoice)&&(!ae_OnOff))
	{
		b1levelval *= noteOnOffFade;
		b2levelval *= noteOnOffFade;
	}

	Env1temp = Env1;	//don't really need this one?	
	Env2temp = Env2;
	LFO1temp = LFO1;
	LFO2temp = LFO2;

	//Now work out the modulation for Buffer2 (do this one first because it can also modulate Buffer1)

	//envelope2
	switch(e2_Destination)
	{
		case dest_buf1level:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b1levelval = Env2temp + (b1levelval * (1.0f-e2_ModDepth));
			break;
		case dest_buf1speed:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b1speedval = Env2temp + (b1speedval * (1.0f-e2_ModDepth));
			break;
		case dest_buf1size:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			if(b1_FromStart)
			{
				Env2temp *= (BUFFERSIZE-2-b1startval);
				b1endval -= b1startval;
				b1endval = b1startval + (Env2temp + (b1endval * (1.0f-e2_ModDepth)));
			}
			else
			{
				Env2temp *= (b1endval-2);
				b1startval = Env2temp + (b1startval * (1.0f-e2_ModDepth));
			}
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			if((b1endval-1) > (BUFFERSIZE-1))
				b1endval = BUFFERSIZE - 2;
			break;
		case dest_buf1start:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp -= 0.5f;
			Env2temp *= e2_ModDepth;
			if((b1endval-2-b1startval) > b1startval)
				Env2temp *= b1startval;
			else
				Env2temp *= (b1endval-2-b1startval);
			b1startval += Env2temp;
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			break;
		case dest_buf1end:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp -= 0.5f;
			Env2temp *= e2_ModDepth;
			if((b1endval-1-b1startval) > ((float)BUFFERSIZE-1-b1endval-1))
				Env2temp *= (BUFFERSIZE-1-b1endval-1);
			else
				Env2temp *= (b1endval-1-b1startval);
			b1endval += Env2temp;
			if((b1endval-1) < (b1startval+1))
				b1endval = b1startval + 3.0f;
			break;
		case dest_buf2level:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b2levelval = Env2temp + (b2levelval * (1.0f-e2_ModDepth));
			break;
		case dest_buf2speed:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b2speedval = Env2temp + (b2speedval * (1.0f-e2_ModDepth));
			break;
		case dest_buf2size:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			if(b2_FromStart)
			{
				Env2temp *= (BUFFERSIZE-2-b2startval);
				b2endval -= b2startval;
				b2endval = b2startval + (Env2temp + (b2endval * (1.0f-e2_ModDepth)));
			}
			else
			{
				Env2temp *= (b2endval-2);
				b2startval = Env2temp + (b2startval * (1.0f-e2_ModDepth));
			}
			if((b2endval-2) < (b2startval+1))
				b2endval = b2startval + 4.0f;
			if((b2endval-1) > (BUFFERSIZE-1))
				b2endval = BUFFERSIZE - 2;
			break;
		case dest_buf2start:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp -= 0.5f;
			Env2temp *= e2_ModDepth;
			if((b2endval-2-b2startval) > b2startval)
				Env2temp *= b2startval;
			else
				Env2temp *= (b2endval-2-b2startval);
			b2startval += Env2temp;
			if((b2endval-2) < (b2startval+1))
				b2endval = b2startval + 4.0f;
			break;
		case dest_buf2end:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp -= 0.5f;
			Env2temp *= e2_ModDepth;
			if((b2endval-1-b2startval) > ((float)BUFFERSIZE-1-b2endval-1))
				Env2temp *= (BUFFERSIZE-1-b2endval-1);
			else
				Env2temp *= (b2endval-1-b2startval);
			b2endval += Env2temp;
			if((b2endval-1) < (b2startval+1))
				b2endval = b2startval + 3.0f;
			break;
		case dest_filtcutoff:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			Env2temp *= FILTER_MAX;
			filtcutoffval = Env2temp + (filtcutoffval * (1.0f-e2_ModDepth));
			break;
		case dest_filtresonance:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			filtresval = Env2temp + (filtresval * (1.0f-e2_ModDepth));
			break;
		case dest_lfo1depth:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			lfo1depthval = Env2temp + (lfo1depthval * (1.0f-e2_ModDepth));
			break;
		case dest_lfo2depth:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			lfo2depthval = Env2temp + (lfo2depthval * (1.0f-e2_ModDepth));
			break;
		case dest_buf1readp:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b1readpval = Env2temp * (b1endval-1-b1startval);
			Index1 -= b1startval;
			Index1 = b1startval + (b1readpval +((1.0f-e2_ModDepth)*Index1));
			break;
		case dest_buf2readp:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b2readpval = Env2temp * (b2endval-1-b2startval);
			Index2 -= b2startval;
			Index2 = b2startval + (b2readpval +((1.0f-e2_ModDepth)*Index2));
			break;
		case dest_buf1pan:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b1panval = Env2temp + (b1panval * (1.0f-e2_ModDepth));
			//SetParameter(kb1_Pan, b1panval);
			break;
		case dest_buf2pan:
			if(e2_InvertEnvelope)
			{
				Env2temp *= -1.0f;
				Env2temp += 1.0f;
			}
			Env2temp *= e2_ModDepth;
			b2panval = Env2temp + (b2panval * (1.0f-e2_ModDepth));
			//SetParameter(kb2_Pan, b2panval);
			break;
	}

	//LFO1
	switch(lfo1_Destination)
	{
		case dest_buf1level:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b1levelval = LFO1temp + (b1levelval * (1.0f-lfo1depthval));
			break;
		case dest_buf1speed:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b1speedval = LFO1temp + (b1speedval * (1.0f-lfo1depthval));
			break;
		case dest_buf1size:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			if(b1_FromStart)
			{
				LFO1temp *= (BUFFERSIZE-2-b1startval);
				b1endval -= b1startval;
				b1endval = b1startval + (LFO1temp + (b1endval * (1.0f-lfo1depthval)));
			}
			else
			{
				LFO1temp *= (b1endval-2);
				b1startval = LFO1temp + (b1startval * (1.0f-lfo1depthval));
			}
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			if((b1endval-1) > (BUFFERSIZE-1))
				b1endval = BUFFERSIZE - 2;
			break;
		case dest_buf1start:
			LFO1temp *= 0.5f; //because the following value is for 0->1
			LFO1temp *= lfo1depthval;
			if((b1endval-2-b1startval) > b1startval)
				LFO1temp *= b1startval;
			else
				LFO1temp *= (b1endval-2-b1startval);
			b1startval += LFO1temp;
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			break;
		case dest_buf1end:
			LFO1temp *= 0.5f; //because the following value is for 0->1
			LFO1temp *= lfo1depthval;
			if((b1endval-1-b1startval) > ((float)BUFFERSIZE-1-b1endval-1))
				LFO1temp *= (BUFFERSIZE-1-b1endval-1);
			else
				LFO1temp *= (b1endval-1-b1startval);
			b1endval += LFO1temp;
			if((b1endval-1) < (b1startval+1))
				b1endval = b1startval + 3.0f;
			break;
		case dest_buf2level:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b2levelval = LFO1temp + (b2levelval * (1.0f-lfo1depthval));
			break;
		case dest_buf2speed:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b2speedval = LFO1temp + (b2speedval * (1.0f-lfo1depthval));
			break;
		case dest_buf2size:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			if(b2_FromStart)
			{
				LFO1temp *= (BUFFERSIZE-2-b2startval);
				b2endval -= b2startval;
				b2endval = b2startval + (LFO1temp + (b2endval * (1.0f-lfo1depthval)));
			}
			else
			{
				LFO1temp *= (b2endval-2);
				b2startval = LFO1temp + (b2startval * (1.0f-lfo1depthval));
			}
			if((b2endval-2) < (b2startval+1))
				b2endval = b2startval + 4.0f;
			if((b2endval-1) > (BUFFERSIZE-1))
				b2endval = BUFFERSIZE - 2;
			break;
		case dest_buf2start:
			LFO1temp *= 0.5f; //because the following value is for 0->1
			LFO1temp *= lfo1depthval;
			if((b2endval-2-b2startval) > b2startval)
				LFO1temp *= b2startval;
			else
				LFO1temp *= (b2endval-2-b2startval);
			b2startval += LFO1temp;
			if((b2endval-2) < (b2startval+1))
				b2endval = b2startval + 4.0f;
			break;
		case dest_buf2end:
			LFO1temp *= 0.5f; //because the following value is for 0->1
			LFO1temp *= lfo1depthval;
			if((b2endval-1-b2startval) > ((float)BUFFERSIZE-1-b2endval-1))
				LFO1temp *= (BUFFERSIZE-1-b2endval-1);
			else
				LFO1temp *= (b2endval-1-b2startval);
			b2endval += LFO1temp;
			if((b2endval-1) < (b2startval+1))
				b2endval = b2startval + 3.0f;
			break;
		case dest_filtcutoff:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			LFO1temp *= FILTER_MAX;
			filtcutoffval = LFO1temp + (filtcutoffval * (1.0f-lfo1depthval));
			break;
		case dest_filtresonance:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			filtresval = LFO1temp + (filtresval * (1.0f-lfo1depthval));
			break;
		case dest_buf1readp:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b1readpval = LFO1temp * (b1endval-1-b1startval);
			Index1 -= b1startval;
			Index1 = b1startval + (b1readpval +((1.0f-lfo1depthval)*Index1));
			break;
		case dest_buf2readp:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b2readpval = LFO1temp * (b2endval-1-b2startval);
			Index2 -= b2startval;
			Index2 = b2startval + (b2readpval +((1.0f-lfo1depthval)*Index2));
			break;
		case dest_buf1pan:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b1panval = LFO1temp + (b1panval * (1.0f-lfo1depthval));
			//SetParameter(kb1_Pan, b1panval);
			break;
		case dest_buf2pan:
			LFO1temp *= 0.5f;
			LFO1temp += 0.5f;
			LFO1temp *= lfo1depthval;
			b2panval = LFO1temp + (b2panval * (1.0f-lfo1depthval));
			//SetParameter(kb2_Pan, b2panval);
			break;
	}

	//LFO2
	switch(lfo2_Destination)
	{
		case dest_buf1level:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b1levelval = LFO2temp + (b1levelval * (1.0f-lfo2depthval));
			break;
		case dest_buf1speed:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b1speedval = LFO2temp + (b1speedval * (1.0f-lfo2depthval));
			break;
		case dest_buf1size:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			if(b1_FromStart)
			{
				LFO2temp *= (BUFFERSIZE-2-b1startval);
				b1endval -= b1startval;
				b1endval = b1startval + (LFO2temp + (b1endval * (1.0f-lfo2depthval)));
			}
			else
			{
				LFO2temp *= (b1endval-2);
				b1startval = LFO2temp + (b1startval * (1.0f-lfo2depthval));
			}
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			if((b1endval-1) > (BUFFERSIZE-1))
				b1endval = BUFFERSIZE - 2;
			break;
		case dest_buf1start:
			LFO2temp *= 0.5f; //because the following value is for 0->1
			LFO2temp *= lfo2depthval;
			if((b1endval-2-b1startval) > b1startval)
				LFO2temp *= b1startval;
			else
				LFO2temp *= (b1endval-2-b1startval);
			b1startval += LFO2temp;
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			break;
		case dest_buf1end:
			LFO2temp *= 0.5f; //because the following value is for 0->1
			LFO2temp *= lfo2depthval;
			if((b1endval-1-b1startval) > ((float)BUFFERSIZE-1-b1endval-1))
				LFO2temp *= (BUFFERSIZE-1-b1endval-1);
			else
				LFO2temp *= (b1endval-1-b1startval);
			b1endval += LFO2temp;
			if((b1endval-1) < (b1startval+1))
				b1endval = b1startval + 3.0f;
			break;
		case dest_buf2level:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b2levelval = LFO2temp + (b2levelval * (1.0f-lfo2depthval));
			break;
		case dest_buf2speed:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b2speedval = LFO1temp + (b2speedval * (1.0f-lfo2depthval));
			break;
		case dest_buf2size:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			if(b2_FromStart)
			{
				LFO2temp *= (BUFFERSIZE-2-b2startval);
				b2endval -= b2startval;
				b2endval = b2startval + (LFO2temp + (b2endval * (1.0f-lfo2depthval)));
			}
			else
			{
				LFO2temp *= (b2endval-2);
				b2startval = LFO2temp + (b2startval * (1.0f-lfo2depthval));
			}
			if((b2endval-2) < (b2startval+1))
				b2endval = b2startval + 4.0f;
			if((b2endval-1) > (BUFFERSIZE-1))
				b2endval = BUFFERSIZE - 2;
			break;
		case dest_buf2start:
			LFO2temp *= 0.5f; //because the following value is for 0->1
			LFO2temp *= lfo2depthval;
			if((b2endval-2-b2startval) > b2startval)
				LFO2temp *= b2startval;
			else
				LFO2temp *= (b2endval-2-b2startval);
			b2startval += LFO2temp;
			if((b2endval-2) < (b2startval+1))
				b2endval = b2startval + 4.0f;
			break;
		case dest_buf2end:
			LFO2temp *= 0.5f; //because the following value is for 0->1
			LFO2temp *= lfo2depthval;
			if((b2endval-1-b2startval) > ((float)BUFFERSIZE-1-b2endval-1))
				LFO2temp *= (BUFFERSIZE-1-b2endval-1);
			else
				LFO2temp *= (b2endval-1-b2startval);
			b2endval += LFO2temp;
			if((b2endval-1) < (b2startval+1))
				b2endval = b2startval + 3.0f;
			break;
		case dest_filtcutoff:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			LFO2temp *= FILTER_MAX;
			filtcutoffval = LFO2temp + (filtcutoffval * (1.0f-lfo2depthval));
			break;
		case dest_filtresonance:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			filtresval = LFO2temp + (filtresval * (1.0f-lfo2depthval));
			break;
		case dest_buf1readp:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b1readpval = LFO2temp * (b1endval-1-b1startval);
			Index1 -= b1startval;
			Index1 = b1startval + (b1readpval +((1.0f-lfo2depthval)*Index1));
			break;
		case dest_buf2readp:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b2readpval = LFO2temp * (b2endval-1-b2startval);
			Index2 -= b2startval;
			Index2 = b2startval + (b2readpval +((1.0f-lfo2depthval)*Index2));
			break;
		case dest_buf1pan:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b1panval = LFO2temp + (b1panval * (1.0f-lfo2depthval));
			//SetParameter(kb1_Pan, b1panval);
			break;
		case dest_buf2pan:
			LFO2temp *= 0.5f;
			LFO2temp += 0.5f;
			LFO2temp *= lfo2depthval;
			b2panval = LFO2temp + (b2panval * (1.0f-lfo2depthval));
			//SetParameter(kb2_Pan, b2panval);
			break;
	}

	//Buffer 2
	if((b2_Level > 0.0f)||(b2_ModDepth > 0.0f))	//i.e. we don't have to calculate anything if it's not going to output to anything
	{
		//calculate the increment for Buffer 2
		if(b2speedval == 0.5f)
			b2speedval = 1.0f;
		else if(b2speedval < 0.5f)
		{
			b2speedval *= 1.5f;	//val = 0->0.75
			b2speedval += 0.25f;//val = 0->1 (effectively 0.25->1)
		}
		else if(b2speedval <= 1.0f)
		{
			b2speedval -= 0.5f;
			b2speedval *= 6.0f;
			b2speedval += 1.0f;
		}
		else
			b2speedval = 1.0f;

		/*if(OscillatorMode)
		{
			b2speedval *= BaseFrequency;
			Inc2temp = (b2endval-b2startval)*InverseSampleRate;	//= no. samples to skip from the table for one cycle to equal one second
			Inc2temp = Inc2temp * b2speedval;
		}
		else
			Inc2temp = b2speedval;	//i.e. we're not concerned with frequency, 4x = skip 4 samples each time*/

		Cond2temp1 = (b2speedval * (1.0f-OscMode_f));	//this stuff's in place of the above, doing it this way saves a little speed

		//b2speedval *= BaseFrequency;
		Cond2temp2 = (b2endval-b2startval)*InverseSampleRate;
		Cond2temp2 *= ((b2speedval*BaseFrequency)*OscMode_f);

		//Inc2temp = (Cond2temp2*OscMode_f) + (Cond2temp1*(1.0f-OscMode_f));
		Inc2temp = Cond2temp2 + Cond2temp1;

		if(!OscillatorMode)
		{
			if(Index2 < (b2startval+8.0f))
				Fades2 = 0.125f * (float)float2int(Index2-b2startval);
			else if(Index2 > (b2endval-8.0f))
				Fades2 = 0.125f * (float)float2int(b2endval-Index2);
			else
				Fades2 = 1.0f;
		}

		if(!b2_Reverse)	//i.e. we're playing forward
		{
			//Have to check before we read, because of the chance for modulation...
			if(Index2 > b2endval)
			{
				if(b2speedval == 1.0f)
					Index2 = b2startval;
				else
				{
					//temp1 = (Index2-(b2endval-1.0f)); //move temp1 back within the buffer
					temp1 = (Index2-b2endval); //move temp1 back within the buffer
					temp2 = (temp1-(float)float2int(temp1)); //temp2 = fractional part of temp1
					//temp1 = (float)(float2int(temp1) % float2int((b2endval-1.0f) - b2startval));//
					temp1 = (float)(float2int(temp1) % float2int(b2endval - b2startval));//
					temp1 += b2startval;
					temp1 += temp2;
					Index2 = temp1;
				}
			}
			else if(Index2 < b2startval)
				Index2 = b2startval;

			val1 = float2int(Index2);
			if(b2_LinearInterp)
			{
				
				val2 = val1 + 1;
				if(val2 > (b2endval-1))
					//val2 = (unsigned long)(b2endval-1);
					val2 = static_cast<unsigned long>(b2startval); //this makes more sense, really...
				Buff2l = interp(Buffer2[val1], Buffer2[val2], Index2);
			}
			else
			{
				Buff2l = Buffer2[val1];
			}

			Index2 += Inc2temp;

			/*if((b2_MovingRP)&&(!OscillatorMode)&&(!b2_Reverse)&&(b2_Speed==0.5f))
			{
				if(NoteMaster->IPBuff2Index > b2_ReadPosition)
					b2_gaptemp = float2int(NoteMaster->IPBuff2Index-b2_ReadPosition);
				else
					b2_gaptemp = float2int((NoteMaster->IPBuff2Index+(BUFFERSIZE-1))-b2_ReadPosition);
				if(NoteMaster->IPBuff2Index > Index2)
				{
					if(float2int(NoteMaster->IPBuff2Index-Index2) < b2_gaptemp)
					{
						Index2 -= 2.0f;	//i.e. 1 sample forward, 2 samples back
						if(Index2 < 0.0f)
							Index2 = (float)(BUFFERSIZE-1);
					}
					else
					{
						Index2 += 1.0f;
					}

					if(float2int(NoteMaster->IPBuff2Index-Index2) != b2_gaptemp)
						b2_MovingRP = true;
					else
						b2_MovingRP = false;
				}
				else
				{
					if(float2int(((float)(BUFFERSIZE-1)-Index2)+NoteMaster->IPBuff2Index) < b2_gaptemp)
					{
						Index2 -= 2.0f;	//i.e. 1 sample forward, 2 samples back
						if(Index2 < 0.0f)
							Index2 = (float)(BUFFERSIZE-1);
					}
					else
					{
						Index2 += 1.0f;
					}
					if(float2int(((float)(BUFFERSIZE-1)-Index2)+NoteMaster->IPBuff2Index) != b2_gaptemp)
						b2_MovingRP = true;
					else
						b2_MovingRP = false;
				}
			}*/

			/*if(Index2 > (b2endval-1))
			{
				temp1 = (Index2-(b2endval-1)); //move temp1 back within the buffer
				temp2 = (temp1-(float)float2int(temp1)); //temp2 = fractional part of temp1
				temp1 = (float)(float2int(temp1) % float2int((b2endval-1) - b2startval));//
				temp1 += b2startval;
				temp1 += temp2;
				Index2 = temp1;
			}*/
		}
		else //reverse
		{
			if(Index2 < (b2startval+1))
			{
				temp1 = Index2;
				temp2 = (temp1-(float)float2int(temp1));
				temp1 = (float)(float2int(temp1) % float2int((b2endval-1) - b2startval));
				if(temp1 < 0.0f)
					temp1 += (b2endval-1);
				else
					temp1 = (b2endval-1) - temp1;
				temp1 += temp2;	//temp2 is a negative value here...
				Index2 = temp1;
			}
			else if(Index2 > (b2endval-1))
				Index2 = b2endval-1;

			val1 = float2int(Index2);
			if(b2_LinearInterp)
			{
				
				val2 = val1 + 1;
				if(val2 > (b2endval-1))
					//val2 = (unsigned long)(b2endval-1);
					val2 = static_cast<unsigned long>(b2startval);
				Buff2l = interp(Buffer2[val1], Buffer2[val2], Index2);
			}
			else
			{
				Buff2l = Buffer2[val1];
			}
			Index2 -= Inc2temp;
			/*if(Index2 < (b2startval+1))
			{
				temp1 = Index2;
				temp2 = (temp1-(float)float2int(temp1));
				temp1 = (float)(float2int(temp1) % float2int((b2endval-1) - b2startval));
				if(temp1 < 0.0f)
					temp1 += (b2endval-1);
				else
					temp1 = (b2endval-1) - temp1;
				temp1 += temp2;	//temp2 is a negative value here...
				Index2 = temp1;
			}*/
		}
		if(PitchCorrection == 1) //brutal
		{
			if(Index2 < (((End2-Start2)*0.5f)+Start2))
				Buff2l = (float)fabs(Buff2l);
			else
				Buff2l = -1.0f * (float)fabs(Buff2l);
		}
		else if(PitchCorrection == 2) //nice
		{
			//Buff2l = (float)fabs(Buff2l);
			Buff2l *= 0.5f;
			Buff2l += 0.5f;
			Buff2l *= sinf((Index2/(End2-Start2))*TWO_PI);
		}
		Buff2l *= Fades2;
		Buff2temp = Buff2l;
		
		if((b2_Freeze&&b2_OnlyOPWhenFrozen)||(!b2_OnlyOPWhenFrozen))
			Buff2l *= b2levelval;	//the audio output of the buffer
		else
			Buff2l = 0.0f;

		if(b2_Envelope)
		{
			if(fabs(Buff2temp) > Env)
				Env = (float)fabs(Buff2temp);
			else
				Env *= 0.999748572451f;
			Buff2temp = (Env-0.5f)*2.0f;	//because the following code is expecting a value -1>1
		}

		Buff2r = Buff2l * b2panval;
		Buff2l *= (1.0f-b2panval);
	}

	//calculating Buffer 2's modulating signal
	switch(b2_Destination)
	{
		case dest_buf1level:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			b1levelval = Buff2temp + (b1levelval * (1.0f-b2_ModDepth));
			break;
		case dest_buf1speed:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			b1speedval = Buff2temp + (b1speedval * (1.0f-b2_ModDepth));
			break;
		case dest_buf1size:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			if(b1_FromStart)
			{
				Buff2temp *= (BUFFERSIZE-2-b1startval);
				b1endval -= b1startval;
				b1endval = b1startval + (Buff2temp + (b1endval * (1.0f-b2_ModDepth)));
			}
			else
			{
				Buff2temp *= (b1endval-2);
				b1startval = Buff2temp + (b1startval * (1.0f-b2_ModDepth));
			}
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			if((b1endval-1) > (BUFFERSIZE-1))
				b1endval = BUFFERSIZE - 2;
			break;
		case dest_buf1start:
			Buff2temp *= 0.5f;
			Buff2temp *= b2_ModDepth;
			if((b1endval-2-b1startval) > b1startval)
				Buff2temp *= b1startval;
			else
				Buff2temp *= (b1endval-2-b1startval);
			b1startval += Buff2temp;
			if((b1endval-2) < (b1startval+1))
				b1endval = b1startval + 4.0f;
			break;
		case dest_buf1end:
			Buff2temp *= 0.5f;
			Buff2temp *= b2_ModDepth;
			if((b1endval-1-b1startval) > ((float)BUFFERSIZE-1-b1endval-1))
				Buff2temp *= (BUFFERSIZE-1-b1endval-1);
			else
				Buff2temp *= (b1endval-1-b1startval);
			b1endval += Buff2temp;
			if((b1endval-1) < (b1startval+1))
				b1endval = b1startval + 3.0f;
			break;
		case dest_filtcutoff:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			Buff2temp *= FILTER_MAX;
			filtcutoffval = Buff2temp + (filtcutoffval * (1.0f-b2_ModDepth));
			break;
		case dest_filtresonance:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			filtresval = Buff2temp + (filtresval * (1.0f-b2_ModDepth));
			break;
		case dest_buf1readp:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			b1readpval = Buff2temp * (b1endval-1-b1startval);
			Index1 -= b1startval;
			Index1 = b1startval + (b1readpval +((1.0f-b2_ModDepth)*Index1));
			break;
		case dest_buf1pan:
			Buff2temp *= 0.5f;
			Buff2temp += 0.5f;
			Buff2temp *= b2_ModDepth;
			b1panval = Buff2temp + (b1panval * (1.0f-b2_ModDepth));
			break;
	}

	//Buffer 1
	if(b1_Level > 0.0f)	//i.e. we don't have to calculate anything if it's not going to output to anything
	{
		//calculate the increment for Buffer 1
		if(b1speedval == 0.5f)
			b1speedval = 1.0f;
		else if(b1speedval < 0.5f)
		{
			b1speedval *= 1.5f;	//val = 0->0.75
			b1speedval += 0.25f;//val = 0->1 (effectively 0.25->1)
		}
		else if(b1speedval <= 1.0f)
		{
			b1speedval -= 0.5f;
			b1speedval *= 6.0f;
			b1speedval += 1.0f;
		}
		else
			b1speedval = 1.0f;

		Cond1temp1 = (b1speedval * (1.0f-OscMode_f));

		//b1speedval *= BaseFrequency;
		Cond1temp2 = (b1endval-b1startval)*InverseSampleRate;
		Cond1temp2 *= ((b1speedval*BaseFrequency)*OscMode_f);

		//Inc1temp = (Cond1temp2*OscMode_f) + (Cond1temp1*(1.0f-OscMode_f));
		Inc1temp = Cond1temp2 + Cond1temp1;

		if(!OscillatorMode)
		{
			if(Index1 < (b1startval+8.0f))
				Fades1 = 0.125f * (float)float2int(Index1-b1startval);
			else if(Index1 > (b1endval-8.0f))
				Fades1 = 0.125f * (float)float2int(b1endval-Index1);
			else
				Fades1 = 1.0f;
		}

		if(!b1_Reverse)	//i.e. we're playing forward
		{
			//if(Index1 > (b1endval-1.0f))
			if(Index1 > b1endval)
			{
				if(b1speedval == 1.0f)
					Index1 = b1startval;
				else
				{
					//temp1 = (Index1-(b1endval-1.0f));
					temp1 = (Index1-b1endval);
					temp2 = (temp1-(float)float2int(temp1)); //it's more lines, but this is faster than fmod()
					//temp1 = (float)(float2int(temp1) % float2int((b1endval-1.0f) - b1startval));
					temp1 = (float)(float2int(temp1) % float2int(b1endval - b1startval));
					temp1 += b1startval;
					temp1 += temp2;
					Index1 = temp1;
				}
			}
			else if(Index1 < b1startval)
				Index1 = b1startval;

			val1 = float2int(Index1);
			if(b1_LinearInterp)
			{
				
				val2 = val1 + 1;
				if(val2 > (b1endval-1))
					//val2 = (unsigned long)(b1endval-1);
					val2 = static_cast<unsigned long>(b1startval); //this makes more sense, really...
				Buff1l = interp(Buffer1[val1], Buffer1[val2], Index1);
			}
			else
			{
				Buff1l = Buffer1[val1];
			}

			Index1 += Inc1temp;
			/*if(Index1 > (b1endval-1))
			{
				temp1 = (Index1-(b1startval-1));
				temp2 = (temp1-(float)float2int(temp1)); //it's more lines, but this is faster than fmod()
				temp1 = (float)(float2int(temp1) % float2int((b1endval-1) - b1startval));
				temp1 += b1startval;
				temp1 += temp2;
				Index1 = temp1;
			}*/
		}
		else //reverse
		{
			if(Index1 < (b1startval+1))
			{
				temp1 = Index1;
				temp2 = (temp1-(float)float2int(temp1));
				temp1 = (float)(float2int(temp1) % float2int((b1endval-1) - b1startval));
				if(temp1 < 0.0f)
					temp1 += (b1endval-1);
				else
					temp1 = (b1endval-1) - temp1;
				temp1 += temp2;	//temp2 is a negative value here...
				Index1 = temp1;
			}
			else if(Index1 > (b1endval-1))
				Index1 = b1endval-1;

			val1 = float2int(Index1);
			if(b1_LinearInterp)
			{
				
				val2 = val1 + 1;
				if(val2 > (b1endval-1))
					//val2 = (unsigned long)(b1endval-1);
					val2 = static_cast<unsigned long>(b1startval);
				Buff1l = interp(Buffer1[val1], Buffer1[val2], Index1);
			}
			else
			{
				Buff1l = Buffer1[val1];
			}
			Index1 -= Inc1temp;
			/*if(Index1 < (b1startval+1))
			{
				temp1 = Index1;
				temp2 = (temp1-(float)float2int(temp1));
				temp1 = (float)(float2int(temp1) % float2int((b1endval-1) - b1startval));
				if(temp1 < 0.0f)
					temp1 += (b1endval-1);
				else
					temp1 = (b1endval-1) - temp1;
				temp1 += temp2;	//temp2 is a negative value here...
				Index1 = temp1;
			}*/
		}
		if(PitchCorrection == 1) //brutal
		{
			if(Index1 < (((End1-Start1)*0.5f)+Start1))
				Buff1l = (float)fabs(Buff1l);
			else
				Buff1l = -1.0f * (float)fabs(Buff1l);
		}
		else if(PitchCorrection == 2) //nice
		{
			//Buff1l = (float)fabs(Buff1l);
			Buff1l *= 0.5f;
			Buff1l += 0.5f;
			Buff1l *= sinf((Index1/(End1-Start1))*TWO_PI);
		}
		Buff1l *= Fades1;

		if((b1_Freeze&&b1_OnlyOPWhenFrozen)||(!b1_OnlyOPWhenFrozen))
			Buff1l *= b1levelval;	//the audio output of the buffer
		else
			Buff1l = 0.0f;

		Buff1r = Buff1l * b1panval;
		Buff1l *= (1.0f-b1panval);
	}

	//Filter
	if((FiltOn)&&((Buff1l+Buff2l)!=0.0f))
	{
		if(filtcutoffval != CutOff)
		{
			if(filtcutoffval <= 0.0f)
				filtcutoffval = 0.5f;
			Filter_CalcF(filtcutoffval);	//could be optimised
		}

		Low = Low + (Band * F);
		High.left = ((Buff1l+Buff2l) * filtresval) - Low.left - (Band.left*filtresval);
		High.right = ((Buff1r+Buff2r) * filtresval) - Low.right - (Band.right*filtresval);
		Band = (High * F) + Band;
		Notch = High + Low;

		switch(Type)
		{
			case ft_High:
				retval = High;
				break;
			case ft_Band:
				retval = Band;
				break;
			case ft_Low:
				retval = Low;
				break;
			case ft_Notch:
				retval = Notch;
				break;
		}

		//ampval = q;
		//ampval = (float)tanh(((1.0f-Q)-1.0f)*4.0f)+1.0f;					//f(x)=tanh((x-1)*4)+1
		//retval = (float)tanh(retval*(1.0f+(ampval*16.0f)));

		float temptwo = ((1.0f-filtresval)*0.5f)*(float)TABLE_SIZE;
		tempIndex = float2int(temptwo);
		ampval = interp(TanhWaveform[tempIndex+1], TanhWaveform[tempIndex], temptwo);
		ampval += 1.0f;

		temptwo = n_tanh(retval.left*(1.0f+(ampval*16.0f)));
		retval.left = temptwo;

		temptwo = n_tanh(retval.right*(1.0f+(ampval*16.0f)));
		retval.right = temptwo;
	}
	else
	{
		retval.left = Buff1l + Buff2l;
		retval.right = Buff1r + Buff2r;
	}

	return retval;
}

//--------------------------------------------------------------------------------------------
//LFO-specific stuff
//--------------------------------------------------------------------------------------------
void bs2voice::LFO1_ResetPhaseTo(float val)
{
	LFO_Index1 = val;
}

//--------------------------------------------------------------------------------------------
void bs2voice::LFO2_ResetPhaseTo(float val)
{
	LFO_Index2 = val;
}

//--------------------------------------------------------------------------------------------
wform bs2voice::Float2Wform(float inval)
{
	wform out;

	if(inval < 0.2f) //sine
			out = wf_sine;
	else if(inval < 0.4f)
		out = wf_saw;
	else if(inval < 0.6f)
		out = wf_squ;
	else if(inval < 0.8f)
		out = wf_sh;
	else
		out = wf_ramp;

	return out;
}

//--------------------------------------------------------------------------------------------
note bs2voice::Float2Note(float inval)
{
	note out;

	if(inval < (1.0f/12.0f))
		out = nl_hemidemisemiquaver;
	else if(inval < (2.0f/12.0f))
		out = nl_demisemiquaver;
	else if(inval < (3.0f/12.0f))
		out = nl_sextuplet;
	else if(inval < (4.0f/12.0f))
		out = nl_semiquaver;
	else if(inval < (5.0f/12.0f))
		out = nl_triplet;
	else if(inval < (6.0f/12.0f))
		out = nl_quaver;
	else if(inval < (7.0f/12.0f))
		out = nl_crotchet;
	else if(inval < (8.0f/12.0f))
		out = nl_minim;
	else if(inval < (9.0f/12.0f))
		out = nl_three;
	else if(inval < (10.0f/12.0f))
		out = nl_semibreve;
	else if(inval < (11.0f/12.0f))
		out = nl_six;
	else
		out = nl_breve;

	return out;
}

//--------------------------------------------------------------------------------------------
float bs2voice::Note2Float(note nlength)
{
	switch(nlength)
	{
		case nl_breve:
			return 8.0f;
		case nl_six:
			return 6.0f;
		case nl_semibreve:
			return 4.0f;
		case nl_three:
			return 3.0f;
		case nl_minim:
			return 2.0f;
		case nl_crotchet:
			return 1.0f;
		case nl_quaver:
			return 0.5f;
		case nl_triplet:
			return (1.0f/3.0f);
		case nl_semiquaver:
			return 0.25f;
		case nl_sextuplet:
			return (1.0f/6.0f);
		case nl_demisemiquaver:
			return 0.125f;
		case nl_hemidemisemiquaver:
			return 0.0625f;
		default:
			return 1.0f;
	}
}

//--------------------------------------------------------------------------------------------
float bs2voice::Note2Float_Inv(note nlength)
{
	switch(nlength)
	{
		case nl_breve:
			return 0.125f;
		case nl_six:
			return (1.0f/6.0f);
		case nl_semibreve:
			return 0.25f;
		case nl_three:
			return (1.0f/3.0f);
		case nl_minim:
			return 0.5f;
		case nl_crotchet:
			return 1.0f;
		case nl_quaver:
			return 2.0f;
		case nl_triplet:
			return 3.0f;
		case nl_semiquaver:
			return 4.0f;
		case nl_sextuplet:
			return 6.0f;
		case nl_demisemiquaver:
			return 8.0f;
		case nl_hemidemisemiquaver:
			return 16.0f;
		default:
			return 1.0f;
	}
}

//--------------------------------------------------------------------------------------------
float bs2voice::CalcFreq(note nlength)
{
	float onebeat, out;

	//onebeat = tempo/60.0f;
	/*onebeat = (float)(Tempo * 0.016666666666666666666666666666667); //0.16... = 1/60

	out = onebeat*Note2Float_Inv(nlength);*/
	onebeat = 60.0f/Tempo;
	out = 1.0f/(onebeat*Note2Float(nlength));

	return out;
}

