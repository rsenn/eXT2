//	BufferSynth2.h - Handles all the VST-specific stuff.
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

#ifndef BUFFERSYNTH2_H_
#define BUFFERSYNTH2_H_

#ifndef __AudioEffectX__
#include <audioeffectx.h>
#endif

#include "bs2PresetData.h"
#include "bs2notemaster.h"
#include "bs2wfl.h"
#include "ParameterFilter.h"

///	Buffer Synth 2 VST plugin class.
class BufferSynth2 : public AudioEffectX
{
  public:
	///	Constructor.
	BufferSynth2(audioMasterCallback audioMaster);
	///	Destructor.
	~BufferSynth2();

	///	Used to make things a little simpler.
	/*!
		I think I borrowed this from Toby Bear's Delphi template?
	 */
#ifdef WIN32
	__forceinline twofloats DoProcess(twofloats a, bool barStart);
#else
	inline twofloats DoProcess(twofloats a, bool barStart);
#endif
	///	Processes the input data in inputs, fills outputs with the plugin's output (accumulating).
	void process(float **inputs, float **outputs, long sampleFrames);
	///	Processes the input data in inputs, fills outputs with the plugin's output (replacing).
	void processReplacing(float **inputs, float **outputs, long sampleFrames);
	///	Called from the host for every (audio) processing block.
	/*!
		Used to handle MIDI data etc.
	 */
	long processEvents(VstEvents* ev);
	///	Should be called by the host before audio processing starts.
	void resume();
	///	Should be called by the host after audio processing stops.
	void suspend();

	///	Fills data with the current preset data as a chunk, to send to the host.
	long getChunk(void** data, bool isPreset);
	/// Sets the plugin's presets according to data.
	long setChunk(void* data, long byteSize, bool isPreset);

	///	Sets the parameters according to the preset index, program.
	void setProgram(long program);
	///	Sets the current program name.
	void setProgramName(char *name);
	///	Fills name with the current program name.
	void getProgramName(char *name);
	///	Fills text with the indexed program name.
	bool getProgramNameIndexed(long category, long index, char* text);
	///	Copies the current program to the program indexed at destination.
	bool copyProgram(long destination);

	///	Sets the indexed parameter to value.
	void setParameter(long index, float value);
	///	Returns the indexed parameter value.
	float getParameter(long index);
	///	Fills label with the indexed parameter's units (e.g. dB etc.).
	void getParameterLabel(long index, char *label);
	///	Fills text with a textual representation of the indexed parameters current value.
	void getParameterDisplay(long index, char *text);
	///	Fills text with the name of the indexed parameter.
	void getParameterName(long index, char *text);
	///	Returns the CC assigned to the indexed parameter (used by BufferSynth2Editor).
	float getCC(long index);
	///	CurrentParam is used by the MIDI Learn stuff to work out which parameter the current MIDI CC should be assigned to.
	/*!
		Used by BufferSynth2Editor.
	 */
	void setCurrentParam(long index) {CurrentParam = index;};
	///	Sets Buffer 1's Size parameter according to it's Start and End parameters.
	void setSize1FromStartEnd(float val);
	///	Sets Buffer 1's Start parameter according to it's End parameter.
	void setStart1FromEnd(float val);
	///	Sets Buffer 1's End parameter according to it's Start parameter.
	void setEnd1FromStart(float val);
	///	Sets Buffer 2's Size parameter according to it's Start and End parameters.
	void setSize2FromStartEnd(float val);
	///	Sets Buffer 2's Start parameter according to it's End parameter.
	void setStart2FromEnd(float val);
	///	Sets Buffer 1's End parameter according to it's Start parameter.
	void setEnd2FromStart(float val);

	///	Called by the host to determine whether the plugin can handle certain operations.
	long canDo (char* text);
	///	Returns the current value of the plugin's vu meter (?).
	float getVu();
	///	Fills name with the plugin's name.
	bool getEffectName(char* name);
	///	Fills text with the name of who made it.
	bool getVendorString(char* text);
	///	Fills text with the name of the plugin.
	bool getProductString(char* text);
	///	Returns the version of the plugin.
	long getVendorVersion();
	///	Returns the plugin's category (see aeffectx.h).
	VstPlugCategory getPlugCategory();
	///	Can be used to tell the host what the various inputs are called.
	bool getInputProperties(long index, VstPinProperties* properties);
	///	Can be used to tell the host what the various outputs are called.
	bool getOutputProperties(long index, VstPinProperties* properties);
	///	Used to tell the host whether audio processing should continue for this plugin after the stop button's been pressed.
	long getTailSize();

	///	Don't use this.  It's stupid.
	bool getBarStart();
	///	Converts value into a textual 'on' or 'off'.
	void onoff2string(float value, char *text);
	///	Converts value into a textual representation of the SizeFrom parameters.
	void sizefrom2string(float value, char *text);
	///	Converts value into a textual representation of the Size2Tempo parameters.
	void size2tempo2string(float value, char *text);
	///	Converts value into a textual representation of the Input parameters.
	void input2string(float value, char *text);
	///	Um... why is this here?.
	void float2string(float value, char *string);
	///	Converts value into a textual representation of Buffer 2's destination parameter.
	void b2dest2string(float value, char *text);
	///	Converts value into a textual representation of Envelope 2's destination parameter.
	void e2dest2string(float value, char *text);
	///	Converts value into a textual representation of Envelope 2's Invert parameter.
	void e2invert2string(float value, char *text);
	///	Converts value into a textual representation of the LFO note length parameters.
	void lfonote2string(float value, char *text);
	///	Converts value into a textual representation of the LFO Destination parameters.
	void lfodest2string(float value, char *text);
	///	Converts value into a textual representation of the Tempo Sync parameters.
	void temposync2string(float value, char *text);
	///	Converts value into a textual representation of the LFO Waveform parameters.
	void wave2string(float value, char *text);
	///	Converts value into a textual representation of the Filter's Type parameter.
	void filttype2string(float value, char *text);
	///	Converts value into a textual representation of the Pitch Correction parameter.
	void pcorrection2string(float value, char *text);
	///	Tells the plugin the size of the editor's wave display widgets.
	void setWDArray(int ArrSize);
	///	Fills buf with the contents of the num buffer.
	void getWDArray(float *buf, int num);
	///	Loads a .wav file into buffer 1.
	void Write2Buffer1(char *path);
	///	Loads a .wav file into buffer 2.
	void Write2Buffer2(char *path);
	///	No idea...
	float GetCS() {return NoteMaster->GetCS();};
	///	Returns the current value of Buffer 1's threshold envelope.
	/*!
		Used by BufferSynth2Editor to flash a wee light when the input
		signal goes above the set threshold.
	 */
	float GetThreshEnv1() {return ThreshEnv1;};
	///	Returns the current value of Buffer 2's threshold envelope.
	/*!
		Used by BufferSynth2Editor to flash a wee light when the input
		signal goes above the set threshold.
	 */
	float GetThreshEnv2() {return ThreshEnv2;};

	///	Loads the (.bsp) patch at path.
	void LoadPatch(char *path);
	///	Saves the (.bsp) patch to path.
	void SavePatch(char *path);
	///	Loads the (.bsb) bank at path.
	void LoadBank(char *path);
	///	Saves the (.bsb) bank to path.
	void SaveBank(char *path);
	///	Moves to the next patch in the bank.
	void IncPatch();
	///	Moves to the previous patch in the bank.
	void DecPatch();

	///	Fills the patches with the preset data.
	void SetPatches();

	//To create internal patches
//#if defined(_WINDOWS) && defined(_DEBUG)
#ifdef TOMASZ_DEBUG
	void Save2Cpp();
#endif

	BS2PresetData *pd;
 
private:
    //BufferSynth2Prog *programs;
    float vu;             //vu meter value
		  //buffer 1
	float fb1_Start,
		  fb1_End,
		  fb1_Size,  // this exists purely to set the start and end points, it doesn't have any direct effect on the plugin
		  fb1_SizeFrom,
		  fb1_RetainSize,
		  fb1_Size2Tempo,
		  fb1_RecThreshold,
		  fb1_Speed_Pitch,
		  fb1_Level,
		  fb1_Input,
		  fb1_StretchFile,
		  fb1_LinearInterp,
		  fb1_Reverse,
		  fb1_OnlyOPWhenFrozen,
		  fb1_MIDINotesSetFreeze,
		  fb1_Freeze,
		  fb1_SizeLessThanMaxFreezes,
		  fb1_InvertSize,
		  fb1_ReadPosition,
		  fb1_ResetRPOnMIDINote,
		  fb1_Pan,
		  fb1_IPGain,

		  //buffer 2
		  fb2_Start,
		  fb2_End,
		  fb2_Size,  // this exists purely to set the start and end points, it doesn't have any direct effect on the plugin
		  fb2_SizeFrom,
		  fb2_RetainSize,
		  fb2_Size2Tempo,
		  fb2_RecThreshold,
		  fb2_Speed_Pitch,
		  fb2_Level,
		  fb2_Input,
		  fb2_StretchFile,
		  fb2_LinearInterp,
		  fb2_Reverse,
		  fb2_OnlyOPWhenFrozen,
		  fb2_MIDINotesSetFreeze,
		  fb2_Freeze,
		  fb2_ModDestination,
		  fb2_ModDepth,
		  fb2_SizeLessThanMaxFreezes,
		  fb2_InvertSize,
		  fb2_Envelope,
		  fb2_ReadPosition,
		  fb2_ResetRPOnMIDINote,
		  fb2_Pan,
		  fb2_IPGain,

		  //amplitude envelope
		  fae_OnOff,
		  fae_Attack,
		  fae_Decay,
		  fae_Sustain,
		  fae_Release,
		  fae_SegmentTime,
		  /*fae_FreezeTriggers,
		  fae_MIDINotesTrigger,*/

		  //second envelope
		  fe2_Attack,
		  fe2_Decay,
		  fe2_Sustain,
		  fe2_Release,
		  fe2_SegmentTime,
		  fe2_MIDINotesTrigger,
		  fe2_BarStartTriggers,
		  fe2_Destination,
		  fe2_Direction,
		  fe2_ModDepth,

		  //LFO1
		  flfo1_Freq_Note,
		  flfo1_TempoSync,
		  flfo1_Waveform,
		  flfo1_BarStartResets,
		  flfo1_MIDINotesReset,
		  flfo1_Destination,
		  flfo1_ModDepth,

		  //LFO2
		  flfo2_Freq_Note,
		  flfo2_TempoSync,
		  flfo2_Waveform,
		  flfo2_BarStartResets,
		  flfo2_MIDINotesReset,
		  flfo2_Destination,
		  flfo2_ModDepth,

		  //Filter
		  ffilt_OnOff,
		  ffilt_Cutoff,
		  ffilt_Resonance,
		  ffilt_Type,

		  //Settings
		  fsett_MIDILearn,
		  fsett_SynthMode,
		  fsett_PolyphonicMode,
		  fsett_PitchCorrection,
		  fsett_SaveBufferContents,

		  //output
		  fop_Mix,
		  fop_Level;

	long CurrentParam;	//used for MIDI Learn

	float samplerate, tempo;

	int checkTooSoon, checkTempo;
	bool checkBSZero;

	bs2notemaster *NoteMaster;
	bs2wfl Loader;

	///	Used to smooth the action of moving the Filter's cutoff parameter.
	/*!
		The thought is that it's this parameter that you're most
		likely to hear quantisation noise on.
	 */
	ParameterFilter *FiltFilter;

	float **Buffer1;
	float **Buffer2;

	int EdArraySize;
	float ThreshEnv1;
	float ThreshEnv2;

	int threshMeterCount;
   
    char kEffectName[24];
    char kProduct[24];
    char kVendor[24];
};

#endif
