//  bs2PresetData.h - The structures and classes we use to store parameter
//					  values etc.
//					  (more than one because we only want to store the buffer
//					  contents if the user specifies it) (huh?)
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

#ifndef BS2PRESETDATA_H_
#define BS2PRESETDATA_H_

#include "EndianSwapFunctions.h"

enum	//simplest here
{
    kNumPrograms = 16,   ///	16 programs
    kNumInputs = 2,      ///	2 inputs
    kNumOutputs = 2,     ///	2 outputs
    kID = 'BSn2',        ///	Unique ID (must be 4 letters long, cannot be the same as another plugin)
    kChannelID = 'BSn2', ///	Channel ID (BufferSynth2, 4 letters, but can be whatever you want) - this is displayed in the Mixer (?)
    kVersionNo = 101,    ///	version number
    kIsSynth = false,    ///	is the plugin a synth?
    kCanReplacing = true,///	can it use processReplacing?
    kCanMono = true,     ///	can it process mono signals
    kVU = false          ///	has it got a vu meter?
};

///	Simple class used to represent a single parameter.
/*!
	Includes the assigned MIDI CC as well as a neat EndianSwap method for
	when the parameter needs to be saved to disk.
 */
class BS2ProgParam
{
  public:
	///	The value of the parameter.
	float val;
	///	The MIDI CC assigned to the parameter.
	/*!
		\todo This probably doesn't need to be an int...
	 */
	int MIDICC;

	///	To make sure the parameter is always saved with the same byte order, regardless of which system it's running on.
	void EndianSwap() {val = LittleFloat(val); MIDICC = LittleShort(MIDICC);};
};

///	Class representing a single preset (i.e. a collection of parameters, some extra odds and ends).
class BS2Preset
{
  public:
				 //buffer 1
	BS2ProgParam b1_Start,
				 b1_End,
				 b1_Size,  // this exists purely to set the start and end points, it doesn't have any direct effect on the plugin
				 b1_SizeFrom,
				 b1_RetainSize,
				 b1_Size2Tempo,
				 b1_RecThreshold,
				 b1_Speed_Pitch,
				 b1_Level,
				 b1_Input,
				 b1_StretchFile,
				 b1_LinearInterp,
				 b1_Reverse,
				 b1_OnlyOPWhenFrozen,
				 b1_MIDINotesSetFreeze,
				 b1_Freeze,
				 b1_SizeLessThanMaxFreezes,
				 b1_InvertSize,
				 b1_ReadPosition,
				 b1_ResetRPOnMIDINote,
				 b1_Pan,
				 b1_IPGain,

				 //buffer 2
				 b2_Start,
				 b2_End,
				 b2_Size,  // this exists purely to set the start and end points, it doesn't have any direct effect on the plugin
				 b2_SizeFrom,
				 b2_RetainSize,
				 b2_Size2Tempo,
				 b2_RecThreshold,
				 b2_Speed_Pitch,
				 b2_Level,
				 b2_Input,
				 b2_StretchFile,
				 b2_LinearInterp,
				 b2_Reverse,
				 b2_OnlyOPWhenFrozen,
				 b2_MIDINotesSetFreeze,
				 b2_Freeze,
				 b2_ModDestination,
				 b2_ModDepth,
				 b2_SizeLessThanMaxFreezes,
				 b2_InvertSize,
				 b2_Envelope,
				 b2_ReadPosition,
				 b2_ResetRPOnMIDINote,
				 b2_Pan,
				 b2_IPGain,

				 //amplitude envelope
				 ae_OnOff,
				 ae_Attack,
				 ae_Decay,
				 ae_Sustain,
				 ae_Release,
				 ae_SegmentTime,
				 /*ae_FreezeTriggers,
				 ae_MIDINotesTrigger,*/

				 //second envelope
				 e2_Attack,
				 e2_Decay,
				 e2_Sustain,
				 e2_Release,
				 e2_SegmentTime,
				 e2_MIDINotesTrigger,
				 e2_BarStartTriggers,
				 e2_Destination,
				 e2_Direction,
				 e2_ModDepth,

				 //LFO1
				 lfo1_Freq_Note,
				 lfo1_TempoSync,
				 lfo1_Waveform,
				 lfo1_BarStartResets,
				 lfo1_MIDINotesReset,
				 lfo1_Destination,
				 lfo1_ModDepth,

				 //LFO2
				 lfo2_Freq_Note,
				 lfo2_TempoSync,
				 lfo2_Waveform,
				 lfo2_BarStartResets,
				 lfo2_MIDINotesReset,
				 lfo2_Destination,
				 lfo2_ModDepth,

				 //Filter
				 filt_OnOff,
				 filt_Cutoff,
				 filt_Resonance,
				 filt_Type,

				 //Settings
				 sett_MIDILearn,
				 sett_SynthMode,
				 sett_PolyphonicMode,
				 sett_PitchCorrection,
				 sett_SaveBufferContents,

				 //output
				 op_Mix,
				 op_Level;

	///	Does this actually get used?
	char b1_filepath[255];
	///	Does this actually get used?
	char b2_filepath[255];

	/// Used so that the host will load the last saved patch, if it exists.
	char patchPath[512];

	//huh?
	/*float *buffer1;
	float *buffer2;*/

	///	The name of the preset.
	char name[24];

	///	Corrects the endian-ness of all the parameters, using their EndianSwap() methods.
	void EndianSwap();
};

///	Represents a bank of presets.
class BS2PresetData
{
friend class BufferSynth2;
public:
	///	Constructor.
	BS2PresetData();
	///	Destructor.
	~BS2PresetData() {};

	///	Corrects the endian-ness of all the presets, using their EndianSwap() methods. 
	void EndianSwap();
private:	
	BS2Preset presets[kNumPrograms];
};

#endif
