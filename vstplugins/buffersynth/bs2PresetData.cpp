//  bs2PresetData.cpp - The structures and classes we use to store parameter
//						values etc.
//						(more than one because we only want to store the
//						buffer contents if the user specifies it) (huh?)
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

#include <stdio.h>
#include "bs2PresetData.h"

//----------------------------------------------------------------------------
//Program Constructor - set default values of the program
//----------------------------------------------------------------------------
BS2PresetData::BS2PresetData()
{
	int i;

	for(i=0;i<kNumPrograms;i++)
	{
		//buffer 1
		presets[i].b1_Start.val = 0.0f;
		presets[i].b1_End.val = 1.0f;
		presets[i].b1_Size.val = 1.0f;
		presets[i].b1_SizeFrom.val = 0.0f;
		presets[i].b1_RetainSize.val = 0.0f;
		presets[i].b1_Size2Tempo.val = 0.0f;
		presets[i].b1_RecThreshold.val = 0.0f;
		presets[i].b1_Speed_Pitch.val = 0.5f;
		presets[i].b1_Level.val = 0.5f;
		presets[i].b1_Input.val = 0.0f;
		presets[i].b1_StretchFile.val = 0.0f;
		presets[i].b1_LinearInterp.val = 1.0f;
		presets[i].b1_Reverse.val = 0.0f;
		presets[i].b1_OnlyOPWhenFrozen.val = 0.0f;
		presets[i].b1_MIDINotesSetFreeze.val = 0.0f;
		presets[i].b1_Freeze.val = 0.0f;
		presets[i].b1_SizeLessThanMaxFreezes.val = 0.0f;
		presets[i].b1_InvertSize.val = 0.0f;
		presets[i].b1_ReadPosition.val = 0.0f;
		presets[i].b1_ResetRPOnMIDINote.val = 0.0f;
		presets[i].b1_Pan.val = 0.5f;
		presets[i].b1_IPGain.val = 0.5f;
	
		//buffer 2
		presets[i].b2_Start.val = 0.0f;
		presets[i].b2_End.val = 1.0f;
		presets[i].b2_Size.val = 1.0f;
		presets[i].b2_SizeFrom.val = 0.0f;
		presets[i].b2_RetainSize.val = 0.0f;
		presets[i].b2_Size2Tempo.val = 0.0f;
		presets[i].b2_RecThreshold.val = 0.0f;
		presets[i].b2_Speed_Pitch.val = 0.5f;
		presets[i].b2_Level.val = 0.5f;
		presets[i].b2_Input.val = 0.5f;
		presets[i].b2_StretchFile.val = 0.0f;
		presets[i].b2_LinearInterp.val = 1.0f;
		presets[i].b2_Reverse.val = 0.0f;
		presets[i].b2_OnlyOPWhenFrozen.val = 0.0f;
		presets[i].b2_MIDINotesSetFreeze.val = 0.0f;
		presets[i].b2_Freeze.val = 0.0f;
		presets[i].b2_ModDestination.val = 0.0f;
		presets[i].b2_ModDepth.val = 1.0f;
		presets[i].b2_SizeLessThanMaxFreezes.val = 0.0f;
		presets[i].b2_InvertSize.val = 0.0f;
		presets[i].b2_Envelope.val = 0.0f;
		presets[i].b2_ReadPosition.val = 0.0f;
		presets[i].b2_ResetRPOnMIDINote.val = 0.0f;
		presets[i].b2_Pan.val = 0.5f;
		presets[i].b2_IPGain.val = 0.5f;

		//amplitude envelope
		presets[i].ae_OnOff.val = 0.0f;
		presets[i].ae_Attack.val = 1.0f;
		presets[i].ae_Decay.val = 1.0f;
		presets[i].ae_Sustain.val = 0.5f;
		presets[i].ae_Release.val = 1.0f;
		presets[i].ae_SegmentTime.val = 0.1f;
		/*presets[i].ae_FreezeTriggers.val = 0.0f;
		presets[i].ae_MIDINotesTrigger.val = 1.0f;*/

		//second envelope
		presets[i].e2_Attack.val = 1.0f;
		presets[i].e2_Decay.val = 1.0f;
		presets[i].e2_Sustain.val = 0.5f;
		presets[i].e2_Release.val = 1.0f;
		presets[i].e2_SegmentTime.val = 0.1f;
		presets[i].e2_MIDINotesTrigger.val = 1.0f;
		presets[i].e2_BarStartTriggers.val = 0.0f;
		presets[i].e2_Destination.val = 0.0f;
		presets[i].e2_Direction.val = 0.0f;
		presets[i].e2_ModDepth.val = 1.0f;

		//LFO1
		presets[i].lfo1_Freq_Note.val = 0.5f;
		presets[i].lfo1_TempoSync.val = 0.0f;
		presets[i].lfo1_Waveform.val = 0.0f;
		presets[i].lfo1_BarStartResets.val = 0.0f;
		presets[i].lfo1_MIDINotesReset.val = 0.0f;
		presets[i].lfo1_Destination.val = 0.0f;
		presets[i].lfo1_ModDepth.val = 1.0f;

		//LFO2
		presets[i].lfo2_Freq_Note.val = 0.5f;
		presets[i].lfo2_TempoSync.val = 0.0f;
		presets[i].lfo2_Waveform.val = 0.0f;
		presets[i].lfo2_BarStartResets.val = 0.0f;
		presets[i].lfo2_MIDINotesReset.val = 0.0f;
		presets[i].lfo2_Destination.val = 0.0f;
		presets[i].lfo2_ModDepth.val = 1.0f;

		//Filter
		presets[i].filt_OnOff.val = 0.0f;
		presets[i].filt_Cutoff.val = 1.0f;
		presets[i].filt_Resonance.val = 0.0f;
		presets[i].filt_Type.val = 0.0f;

		//Settings
		presets[i].sett_MIDILearn.val = 0.0f;
		presets[i].sett_SynthMode.val = 0.0f;
		presets[i].sett_PolyphonicMode.val = 0.0f;
		presets[i].sett_PitchCorrection.val = 0.0f;	
		presets[i].sett_SaveBufferContents.val = 0.0f;

		//output
		presets[i].op_Mix.val = 1.0f;
		presets[i].op_Level.val = 1.0f;

//****--MIDI CCs----------------------------------****

		//buffer 1
		presets[i].b1_Start.MIDICC = 0;
		presets[i].b1_End.MIDICC = 1;
		presets[i].b1_Size.MIDICC = 2;
		presets[i].b1_SizeFrom.MIDICC = 3;
		presets[i].b1_RetainSize.MIDICC = 4;
		presets[i].b1_Size2Tempo.MIDICC = 5;
		presets[i].b1_RecThreshold.MIDICC = 6;
		presets[i].b1_Speed_Pitch.MIDICC = 7;
		presets[i].b1_Level.MIDICC = 8;
		presets[i].b1_Input.MIDICC = 9;
		presets[i].b1_StretchFile.MIDICC = 10;
		presets[i].b1_LinearInterp.MIDICC = 11;
		presets[i].b1_Reverse.MIDICC = 12;
		presets[i].b1_OnlyOPWhenFrozen.MIDICC = 13;
		presets[i].b1_MIDINotesSetFreeze.MIDICC = 14;
		presets[i].b1_Freeze.MIDICC = 15;
		presets[i].b1_SizeLessThanMaxFreezes.MIDICC = 16;
		presets[i].b1_InvertSize.MIDICC = 17;
		presets[i].b1_ReadPosition.MIDICC = 82;
		presets[i].b1_ResetRPOnMIDINote.MIDICC = 83;
		presets[i].b1_Pan.MIDICC = 84;
		presets[i].b1_IPGain.MIDICC = 89;
	
		//buffer 2
		presets[i].b2_Start.MIDICC = 18;
		presets[i].b2_End.MIDICC = 19;
		presets[i].b2_Size.MIDICC = 20;
		presets[i].b2_SizeFrom.MIDICC = 21;
		presets[i].b2_RetainSize.MIDICC = 22;
		presets[i].b2_Size2Tempo.MIDICC = 23;
		presets[i].b2_RecThreshold.MIDICC = 24;
		presets[i].b2_Speed_Pitch.MIDICC = 25;
		presets[i].b2_Level.MIDICC = 26;
		presets[i].b2_Input.MIDICC = 27;
		presets[i].b2_StretchFile.MIDICC = 28;
		presets[i].b2_LinearInterp.MIDICC = 29;
		presets[i].b2_Reverse.MIDICC = 30;
		presets[i].b2_OnlyOPWhenFrozen.MIDICC = 31;
		presets[i].b2_MIDINotesSetFreeze.MIDICC = 32;
		presets[i].b2_Freeze.MIDICC = 33;
		presets[i].b2_ModDestination.MIDICC = 34;
		presets[i].b2_ModDepth.MIDICC = 35;
		presets[i].b2_SizeLessThanMaxFreezes.MIDICC = 36;
		presets[i].b2_InvertSize.MIDICC = 37;
		presets[i].b2_Envelope.MIDICC = 81;
		presets[i].b2_ReadPosition.MIDICC = 85;
		presets[i].b2_ResetRPOnMIDINote.MIDICC = 86;
		presets[i].b2_Pan.MIDICC = 87;
		presets[i].b2_IPGain.MIDICC = 90;

		//amplitude envelope
		presets[i].ae_OnOff.MIDICC = 38;
		presets[i].ae_Attack.MIDICC = 39;
		presets[i].ae_Decay.MIDICC = 40;
		presets[i].ae_Sustain.MIDICC = 41;
		presets[i].ae_Release.MIDICC = 42;
		presets[i].ae_SegmentTime.MIDICC = 43;
		/*presets[i].ae_FreezeTriggers.MIDICC = 44;
		presets[i].ae_MIDINotesTrigger.MIDICC = 45;*/

		//second envelope
		presets[i].e2_Attack.MIDICC = 46;
		presets[i].e2_Decay.MIDICC = 47;
		presets[i].e2_Sustain.MIDICC = 48;
		presets[i].e2_Release.MIDICC = 49;
		presets[i].e2_SegmentTime.MIDICC = 50;
		presets[i].e2_MIDINotesTrigger.MIDICC = 51;
		presets[i].e2_BarStartTriggers.MIDICC = 52;
		presets[i].e2_Destination.MIDICC = 53;
		presets[i].e2_Direction.MIDICC = 54;
		presets[i].e2_ModDepth.MIDICC = 55;

		//LFO1
		presets[i].lfo1_Freq_Note.MIDICC = 56;
		presets[i].lfo1_TempoSync.MIDICC = 57;
		presets[i].lfo1_Waveform.MIDICC = 58;
		presets[i].lfo1_BarStartResets.MIDICC = 59;
		presets[i].lfo1_MIDINotesReset.MIDICC = 60;
		presets[i].lfo1_Destination.MIDICC = 61;
		presets[i].lfo1_ModDepth.MIDICC = 62;

		//LFO2
		presets[i].lfo2_Freq_Note.MIDICC = 63;
		presets[i].lfo2_TempoSync.MIDICC = 64;
		presets[i].lfo2_Waveform.MIDICC = 65;
		presets[i].lfo2_BarStartResets.MIDICC = 66;
		presets[i].lfo2_MIDINotesReset.MIDICC = 67;
		presets[i].lfo2_Destination.MIDICC = 68;
		presets[i].lfo2_ModDepth.MIDICC = 69;

		//Filter
		presets[i].filt_OnOff.MIDICC = 70;
		presets[i].filt_Cutoff.MIDICC = 71;
		presets[i].filt_Resonance.MIDICC = 72;
		presets[i].filt_Type.MIDICC = 73;

		//Settings
		presets[i].sett_MIDILearn.MIDICC = 74;
		presets[i].sett_SynthMode.MIDICC = 75;
		presets[i].sett_PolyphonicMode.MIDICC = 76;
		presets[i].sett_PitchCorrection.MIDICC = 77;	
		presets[i].sett_SaveBufferContents.MIDICC = 78;

		//output
		presets[i].op_Mix.MIDICC = 79;
		presets[i].op_Level.MIDICC = 80;

		//strcpy (presets[i].name, "Init"); //default program name = Init
		sprintf(presets[i].name, "Init %d", i);

		sprintf(presets[i].patchPath, "");
	}
}

void BS2PresetData::EndianSwap()
{
	int i;

	for(i=0;i<kNumPrograms;i++)
	{
		presets[i].EndianSwap();
	}
}

void BS2Preset::EndianSwap()
{
	b1_Start.EndianSwap();
	b1_End.EndianSwap();
	b1_Size.EndianSwap();
	b1_SizeFrom.EndianSwap();
	b1_RetainSize.EndianSwap();
	b1_Size2Tempo.EndianSwap();
	b1_RecThreshold.EndianSwap();
	b1_Speed_Pitch.EndianSwap();
	b1_Level.EndianSwap();
	b1_Input.EndianSwap();
	b1_StretchFile.EndianSwap();
	b1_LinearInterp.EndianSwap();
	b1_Reverse.EndianSwap();
	b1_OnlyOPWhenFrozen.EndianSwap();
	b1_MIDINotesSetFreeze.EndianSwap();
	b1_Freeze.EndianSwap();
	b1_SizeLessThanMaxFreezes.EndianSwap();
	b1_InvertSize.EndianSwap();
	b1_ReadPosition.EndianSwap();
	b1_ResetRPOnMIDINote.EndianSwap();
	b1_Pan.EndianSwap();
	b1_IPGain.EndianSwap();

	b2_Start.EndianSwap();
	b2_End.EndianSwap();
	b2_Size.EndianSwap();
	b2_SizeFrom.EndianSwap();
	b2_RetainSize.EndianSwap();
	b2_Size2Tempo.EndianSwap();
	b2_RecThreshold.EndianSwap();
	b2_Speed_Pitch.EndianSwap();
	b2_Level.EndianSwap();
	b2_Input.EndianSwap();
	b2_StretchFile.EndianSwap();
	b2_LinearInterp.EndianSwap();
	b2_Reverse.EndianSwap();
	b2_OnlyOPWhenFrozen.EndianSwap();
	b2_MIDINotesSetFreeze.EndianSwap();
	b2_Freeze.EndianSwap();
	b2_ModDestination.EndianSwap();
	b2_ModDepth.EndianSwap();
	b2_SizeLessThanMaxFreezes.EndianSwap();
	b2_InvertSize.EndianSwap();
	b2_Envelope.EndianSwap();
	b2_ReadPosition.EndianSwap();
	b2_ResetRPOnMIDINote.EndianSwap();
	b2_Pan.EndianSwap();
	b2_IPGain.EndianSwap();

	ae_OnOff.EndianSwap();
	ae_Attack.EndianSwap();
	ae_Decay.EndianSwap();
	ae_Sustain.EndianSwap();
	ae_Release.EndianSwap();
	ae_SegmentTime.EndianSwap();
	/*ae_FreezeTriggers.EndianSwap();
	ae_MIDINotesTrigger.EndianSwap();*/

	e2_Attack.EndianSwap();
	e2_Decay.EndianSwap();
	e2_Sustain.EndianSwap();
	e2_Release.EndianSwap();
	e2_SegmentTime.EndianSwap();
	e2_MIDINotesTrigger.EndianSwap();
	e2_BarStartTriggers.EndianSwap();
	e2_Destination.EndianSwap();
	e2_Direction.EndianSwap();
	e2_ModDepth.EndianSwap();

	lfo1_Freq_Note.EndianSwap();
	lfo1_TempoSync.EndianSwap();
	lfo1_Waveform.EndianSwap();
	lfo1_BarStartResets.EndianSwap();
	lfo1_MIDINotesReset.EndianSwap();
	lfo1_Destination.EndianSwap();
	lfo1_ModDepth.EndianSwap();

	lfo2_Freq_Note.EndianSwap();
	lfo2_TempoSync.EndianSwap();
	lfo2_Waveform.EndianSwap();
	lfo2_BarStartResets.EndianSwap();
	lfo2_MIDINotesReset.EndianSwap();
	lfo2_Destination.EndianSwap();
	lfo2_ModDepth.EndianSwap();

	filt_OnOff.EndianSwap();
	filt_Cutoff.EndianSwap();
	filt_Resonance.EndianSwap();
	filt_Type.EndianSwap();

	sett_MIDILearn.EndianSwap();
	sett_SynthMode.EndianSwap();
	sett_PolyphonicMode.EndianSwap();
	sett_PitchCorrection.EndianSwap();
	sett_SaveBufferContents.EndianSwap();

	op_Mix.EndianSwap();
	op_Level.EndianSwap();
}
