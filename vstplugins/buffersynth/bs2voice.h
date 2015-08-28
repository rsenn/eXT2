//	bs2voice.h - A single voice of Buffer Synth 2 (it gets called from a
//				 bs2notemaster object).
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

#ifndef BS2VOICE_H_
#define BS2VOICE_H_

#define NDC_PI 3.1415926535897932384626433832795 //I got this from Windows' calculator.

#define BUFFERSIZE 44100	//buffer is 1 second long (at 44.1kHz, may be subject to change)

const double _double2fixmagic = 68719476736.0*1.5;
const long _shiftamt = 16;

#define iexp_ 0
#define iman_ 1

inline int float2int(float val)
{
    return (int)val;
}

///	Simple linear interpolation.
inline float interp(float val1, float val2, float index)
{
	float outval, index_fract;

	index_fract = index - (float)(float2int(index));
	outval = val1 + ((val2 - val1)*index_fract);

	return outval;
}

class bs2notemaster;

///	A new type used to provide the same functionality as a single float, but with two of them.
/*!
	I tend to use this kind of thing a lot now.  \e Very Useful.
 */
struct twofloats
{
  public:
	float left;
	float right;

	twofloats operator+(twofloats op2)	//These operators are so that we can use twofloats
	{									//variables in the filter - it saves having to
		twofloats temp;					//re-write the filter code completely to have 2
		temp.left = op2.left + left;	//separate paths (left & right).
		temp.right = op2.right + right;
		return temp;
	};

	twofloats operator+=(twofloats op2)
	{
		left = op2.left + left;
		right = op2.right + right;
		return *this;
	};

	twofloats operator-(twofloats op2)
	{
		twofloats temp;
		temp.left = left - op2.left;
		temp.right = right - op2.right;
		return temp;
	};

	twofloats operator*(twofloats op2)
	{
		twofloats temp;
		temp.left = op2.left * left;
		temp.right = op2.right * right;
		return temp;
	};

	twofloats operator*(float op2)
	{
		twofloats temp;
		temp.left = op2 * left;
		temp.right = op2 * right;
		return temp;
	};

	twofloats operator*=(float op2)
	{
		left = op2 * left;
		right = op2 * right;
		return *this;
	};
};

///	These are the indices of all the parameters.
enum
{
	//buffer 1
	kb1_Start,
	kb1_End,
	kb1_Size,  // this exists purely to set the start and end points, it doesn't have any direct effect on the plugin
	kb1_SizeFrom,
	kb1_RetainSize,
	kb1_Size2Tempo,
	kb1_RecThreshold,
	kb1_Speed_Pitch,
	kb1_Level,
	kb1_Input,
	kb1_StretchFile,
	kb1_LinearInterp,
	kb1_Reverse,
	kb1_OnlyOPWhenFrozen,
	kb1_MIDINotesSetFreeze,
	kb1_Freeze,
	kb1_SizeLessThanMaxFreezes,
	kb1_InvertSize,
	kb1_ReadPosition,
	kb1_ResetRPOnMIDINote,
	kb1_Pan,
	kb1_IPGain,

	//buffer 2
	kb2_Start,
	kb2_End,
	kb2_Size,  // this exists purely to set the start and end points, it doesn't have any direct effect on the plugin
	kb2_SizeFrom,
	kb2_RetainSize,
	kb2_Size2Tempo,
	kb2_RecThreshold,
	kb2_Speed_Pitch,
	kb2_Level,
	kb2_Input,
	kb2_StretchFile,
	kb2_LinearInterp,
	kb2_Reverse,
	kb2_OnlyOPWhenFrozen,
	kb2_MIDINotesSetFreeze,
	kb2_Freeze,
	kb2_ModDestination,
	kb2_ModDepth,
	kb2_SizeLessThanMaxFreezes,
	kb2_InvertSize,
	kb2_Envelope,
	kb2_ReadPosition,
	kb2_ResetRPOnMIDINote,
	kb2_Pan,
	kb2_IPGain,

	//amplitude envelope
	kae_OnOff,
	kae_Attack,
	kae_Decay,
	kae_Sustain,
	kae_Release,
	kae_SegmentTime,
	//kae_FreezeTriggers,
	//kae_MIDINotesTrigger,

	//second envelope
	ke2_Attack,
	ke2_Decay,
	ke2_Sustain,
	ke2_Release,
	ke2_SegmentTime,
	ke2_MIDINotesTrigger,
	ke2_BarStartTriggers,
	ke2_Destination,
	ke2_Direction,
	ke2_ModDepth,

	//LFO1
	klfo1_Freq_Note,
	klfo1_TempoSync,
	klfo1_Waveform,
	klfo1_BarStartResets,
	klfo1_MIDINotesReset,
	klfo1_Destination,
	klfo1_ModDepth,

	//LFO2
	klfo2_Freq_Note,
	klfo2_TempoSync,
	klfo2_Waveform,
	klfo2_BarStartResets,
	klfo2_MIDINotesReset,
	klfo2_Destination,
	klfo2_ModDepth,

	//Filter
	kfilt_OnOff,
	kfilt_Cutoff,
	kfilt_Resonance,
	kfilt_Type,

	//Settings
	ksett_MIDILearn,
	ksett_SynthMode,
	ksett_PolyphonicMode,
	ksett_PitchCorrection,
	ksett_SaveBufferContents,

	//output
	kop_Mix,
    kop_Level,
    
    kNumParams,        //this is set automatically, depending on the number of parameters you have above it

	kDummy1,	//b1FileLoad
	kDummy2,	//?
	kDummy3,	//b2FileLoad
	kDummy4,	//?
	kDummy5,	//ModKick
	kDummy6,	//e2Kick
	kDummy7,	//lfo1Kick
	kDummy8,	//lfo2Kick
	kDummy9,	//?
	kDummy10,	//patch name
	kDummy11,	//patch dec
	kDummy12,	//patch inc
	kDummy13,	//load patch
	kDummy14,	//save patch
	kDummy15,	//load bank
	kDummy16,	//save bank
	kDummy17,	//random
//#if defined(_WINDOWS) && defined(_DEBUG)
#ifdef TOMASZ_DEBUG
	kDummy18,	//save to internal patch
#endif
	kDummy19,	//threshold lamp, b1
	kDummy20,	//threshold lamp, b2
	kDummy21,	//patch menu
	kDummy22,	//b1s2tkick
	kDummy23,	//b2s2tkick
};

///	All the possible modulation destinations.
enum Destination
{
	dest_off,
	dest_buf1level,
	dest_buf1speed,
	dest_buf1size,
	dest_buf1start,
	dest_buf1end,
	dest_buf2level,
	dest_buf2speed,
	dest_buf2size,
	dest_buf2start,
	dest_buf2end,
	dest_filtcutoff,
	dest_filtresonance,
	dest_lfo1depth,
	dest_lfo2depth,
	dest_buf1pan,
	dest_buf1readp,
	dest_buf2pan,
	dest_buf2readp,
	dest_buf2depth,
	dest_lfo1nfreq,
	dest_lfo2nfreq,
};

//----------------------------------------------------------------------------
//Envelope-specific stuff
//----------------------------------------------------------------------------
enum ADSR
{
	kA,
	kD,
	kS,
	kR,
};

///	TanhWave size?
#define TABLE_SIZE 2048
#define TABLE_SIZE_INT (int)TABLE_SIZE
#define INV_TS (float)(1.0f/(float)TABLE_SIZE)
#define TS_DIV_TWO_PI (TABLE_SIZE/TWO_PI)

//----------------------------------------------------------------------------
//LFO-specific stuff
//----------------------------------------------------------------------------
#ifndef TWO_PI
	#define TWO_PI (2 * NDC_PI)
#endif

#define LFO_MAX_FREQ 30.0f

///	Enumerates the available LFO waveforms.
enum wform {
	wf_sine,           //recosc gets a bit funny at low frequencies, use this instead (?)
	wf_saw,
	wf_squ,
	wf_sh,
	wf_ramp
};

///	Enumerates the available note lengths, when the LFOs are used in tempo sync mode.
enum note {
	nl_breve,			  //8
	nl_six,				  //6
	nl_semibreve,		  //4
	nl_three,			  //3
	nl_minim,			  //2
	nl_crotchet,		  //1
	nl_quaver,			  //1/2
	nl_triplet,			  //1/3
	nl_semiquaver,		  //1/4
	nl_sextuplet,		  //1/6
	nl_demisemiquaver,	  //1/8
	nl_hemidemisemiquaver //1/16
};

//----------------------------------------------------------------------------
//Filter-specific stuff
//----------------------------------------------------------------------------
typedef enum
{
	ft_High,
	ft_Band,
	ft_Low,
	ft_Notch
} filt_type;

///	Maximum frequency the filter will go to.
/*!
	We're using a SV filter, so it can't go to Nyquist w/out getting
	all unstable.
 */
#define FILTER_MAX 7500.0f

//----------------------------------------------------------------------------
//Class Declaration
//----------------------------------------------------------------------------
///	A single voice for the plugin.
/*!
	This is where all the audio stuff happens.
 */
class bs2voice
{
  public:
	///	Constructor (actually, this one's never used...).
	/*!
		\param tag It's index in notemaster's array of voices.
		\param buffer1 pointer to notemaster's buffer1 (this is shared by all the voices).
		\param buffer2 pointer to notemaster's buffer2 (this is shared by all the voices).
		\param tanhwave pointer to notemaster's TanhWave lookup table (this is shared by all the voices).
		\param sinewave pointer to notemaster's SineWave lookup table (this is shared by all the voices).
	 */
	bs2voice(bs2notemaster *notemaster, int tag, float samplerate, float *buffer1, float *buffer2, float *tanhwave, float *sinewave);
	///	Default constructor (for allocating arrays).
	bs2voice() {};
	///	Destructor.
	/*!
		\todo Is this really all that's supposed to happen here?
	 */
	~bs2voice() {/*delete LFO_Filter1; delete LFO_Filter2;*/};
	///	Takes the place of the constructor when the voice is constructed in an array.
	/*!
		\param tag It's index in notemaster's array of voices.
		\param buffer1 pointer to notemaster's buffer1 (this is shared by all the voices).
		\param buffer2 pointer to notemaster's buffer2 (this is shared by all the voices).
		\param tanhwave pointer to notemaster's TanhWave lookup table (this is shared by all the voices).
		\param sinewave pointer to notemaster's SineWave lookup table (this is shared by all the voices).
	 */
	void Create(bs2notemaster *notemaster, int tag, float samplerate, float *buffer1, float *buffer2, float *tanhwave, float *sinewave);

	///	Tells this voice to start platying.
	/*!
		Envelopes go to the start of their Attack phase etc.
	 */
	void NoteOn(int note, float velocity);
	///	Tells the voice to stop playing/go to the envelopes' Release phase.
	void NoteOff();
	///	Sets the pitch-bend value for this voice (does this actually do anything?).
	void SetPitchBend(float val);

	///	Returns whether the voice is currently playing or not.
	bool GetIsActive() {return IsActive;};

	///	Sets the indexed parameter to val.
	void SetParameter(long index, float val);
	//void SetSize2Tempo(note noteval, int buffer);	//time = time the note takes, fromstart => true = set size according to start position
	///	Sets the samplerate.
	void SetSamplerate(float samplerate) {SampleRate = samplerate; InverseSampleRate = 1.0f/SampleRate; Filter_CalcF();};
	///	Sets the tempo.
	/*!
		Used to calculate correct note lengths for the LFOs.
	 */
	void SetTempo(float tempo);

	///	What is this for?
	float GetCS() {return LFO_Increment1;};
	///	Returns the next sample to be sent to the host.
	twofloats GetSample(bool barstart);

	///	Tells the plugin it is the main voice.
	/*!
		The main voice is effectively always on - this allows, for
		example, the use of polyphony even when you're not in synth
		mode.
	 */
	void setFirstVoice() {FirstVoice = true;};
	///	Tells the plugin it's active?
	/*!
		What is this for?
	 */
	void setIsActive(bool val) {IsActive = val;};

  private:
	//------------------------------------------------------------------------
	//Variables
	//------------------------------------------------------------------------
	bs2notemaster *NoteMaster;
	///	The plugin's index in NoteMaster's array of bs2voices
	int Tag;

	///	Used to tell the notemaster whether the voice is already running.
	bool IsActive;
	///	Used so that control signals (i.e. envelopes etc.) are only updated infrequently.
	int ControlCount;

	///	Used to keep track of the current note's frequency.
	float BaseFrequency;
	///	The actual frequency we're using (including pitch bend etc.).
	float Frequency;
	///	The current note's velocity (do we actually do anything with this?).
	float Velocity;
	///	Used to work out pitchbend freq.
	int Note;
	///	Used to know whether to turn off/on polyphonic voices.
	bool FirstVoice;

	float SampleRate;
	///	Used to speed things up a wee bit.
	float InverseSampleRate;

	///	For when the amplitude envelope's not on.
	float noteOnOffFade;

	//------------------------------------------------------------------------
	//Buffer-specific stuff
	//------------------------------------------------------------------------
	float *Buffer1;
	float *Buffer2;

	///	Our current position for reading back from Buffer1.
	float Index1;
	float Increment1;
	///	Our current position for reading back from Buffer2.
	float Index2;
	float Increment2;
	//bool Direction1, Direction2;		//which direction are we reading out of the buffer from (true = forward)
	float Start1, End1;	//start and end points for reading out of the buffers
	float Start2, End2;
	float Frequency1, Frequency2;
	float Env;

	///	?
	float Fades1;
	float Fades2;

	///	true = use buffers as oscillators, & multiply o/p by Velocity (?).
	bool OscillatorMode;
	///	Used as a slight optimisation somewhere?
	float OscMode_f;
	int PitchCorrection;

	float b1_Level;
	float b1_Speed;
	float b2_Level;
	float b2_Speed;
	bool b1_FromStart;
	bool b2_FromStart;
	float b2_ModDepth;
	bool b1_Reverse;
	bool b1_LinearInterp;
	float b1_LInterp_f;
	bool b1_Freeze;
	bool b1_OnlyOPWhenFrozen;
	bool b2_Reverse;
	bool b2_LinearInterp;
	float b2_LInterp_f;
	bool b2_Freeze;
	bool b2_OnlyOPWhenFrozen;
	bool b2_Envelope;

	float b1_ReadPosition;
	bool b1_ResetRPOnMIDINote;
	float b1_Pan;
	float b2_ReadPosition;
	bool b2_ResetRPOnMIDINote;
	float b2_Pan;

	bool b1_RPChanged;
	bool b2_RPChanged;
	float b1_NewRP;
	float b2_NewRP;
	
	Destination b2_Destination;

	//------------------------------------------------------------------------
	//Envelope-specific stuff
	//------------------------------------------------------------------------
	float *TanhWaveform;
	bool NoteIsOff;

	///	The envelopes use a tanh-waveform (s-shaped curve), so they need indices.
	float Index_Env1;
	ADSR CurrentSegment1;
	///	The envelopes use a tanh-waveform (s-shaped curve), so they need indices.
	float Index_Env2;
	ADSR CurrentSegment2;

	float A1,
		  D1,
		  R1,
		  SegmentTime1;	//this one refers to the actual time in seconds, it's not just 0->1
	float IncrementA1,	//****set in SetParameter()****
		  IncrementD1,
		  IncrementR1;
	float A2,
		  D2,
		  R2,
		  SegmentTime2;
	float IncrementA2,
		  IncrementD2,
		  IncrementS2,
		  IncrementR2;

	///	The current value of Envelope 1.
	float Env1;
	///	The current value of Envelope 2.
	float Env2;

	float Tempo_SegmentTime;

	bool ae_OnOff;
	float ae_S;
	float ae_S_actual;
	//bool ae_MIDINotesTrigger;
	Destination e2_Destination;
	float e2_S;
	float e2_S_actual;
	float e2_ModDepth;
	bool e2_InvertEnvelope;
	bool e2_MIDINotesTrigger;
	bool e2_BarStartTriggers;

	//------------------------------------------------------------------------
	//LFO-specific stuff
	//------------------------------------------------------------------------
	///	Resets the oscillator's phase to val.
	/*!
		I'm fairly sure val is only ever 0 here.
	 */
	void LFO1_ResetPhaseTo(float val); //val = 0->1
	///	Resets the oscillator's phase to val.
	/*!
		I'm fairly sure val is only ever 0 here.
	 */
	void LFO2_ResetPhaseTo(float val);
	///	Converts the float value from the plugin into the correct wform enumeration.
	wform Float2Wform(float inval);
	///	Converts the float value from the plugin into the correct note length enumeration.
	note Float2Note(float inval);
	///	Converts a note length enumeration into a float value.
	/*!
		Returns the number of beats.
	 */
	float Note2Float(note nlength);
	///	Converts a note length enumeration into a float value, inverted.
	/*!
		Returns the invers number of beats.  What's this used for
		again?
	 */
	float Note2Float_Inv(note nlength);
	///	Calculates the actual frequency of the oscillator according to nlength and the current tempo.
	float CalcFreq(note nlength);

	/*///	Not used anymore?
	NyquistEllipFilter *LFO_Filter1;
	///	Not used anymore?
	NyquistEllipFilter *LFO_Filter2;*/

	float *SineWaveform;
	float Tempo;
	float LFO_Frequency1;
	float LFO_Frequency2;
	note LFO_Notelength1;
	note LFO_Notelength2;
	wform LFO_Waveform1;
	wform LFO_Waveform2;
	float LFO_Increment1;
	float LFO_Increment2;
	float LFO_Index1;
	float LFO_Index2;

	int BarStartCountDown;
	bool LFO_CurrentVal1;	//used to decide when to update the sample & hold generator
	bool LFO_OldVal1;
	float LFO_Current1;
	bool LFO_CurrentVal2;
	bool LFO_OldVal2;
	float LFO_Current2;

	bool lfo1_TempoSync;
	bool lfo2_TempoSync;
	Destination lfo1_Destination,
				lfo2_Destination;
	float lfo1_ModDepth;
	float lfo2_ModDepth;
	bool lfo1_BarStartResets;
	bool lfo2_BarStartResets;
	bool lfo1_MIDINotesReset;
	bool lfo2_MIDINotesReset;

	float LFO1;			//the outputs of the lfos
	float LFO2;

	//------------------------------------------------------------------------
	//Filter-specific stuff
	//------------------------------------------------------------------------
	///	Calculate the F coefficient of the filter according to the current value of CutOff.
#ifdef WIN32
	__forceinline void Filter_CalcF(){
#else
	inline void Filter_CalcF(){
#endif
		int tempIndex;
		float tempIndex2;
		if(SampleRate < 22050)
			return;
		if(InverseSampleRate < 0)
			return;
		tempIndex2 = ((NDC_PI*CutOff)*InverseSampleRate) * TS_DIV_TWO_PI;
		tempIndex = float2int(tempIndex2);
		F = 2.0f * interp(SineWaveform[tempIndex], SineWaveform[tempIndex+1], tempIndex2);
	};
	///	Calculate the F coefficient of the filter according to val.
#ifdef WIN32
	__forceinline void Filter_CalcF(float val){
#else
	inline  void Filter_CalcF(float val){
#endif
		int tempIndex;
		float tempIndex2;
		tempIndex2 = ((NDC_PI*val)*InverseSampleRate) * TS_DIV_TWO_PI;
		tempIndex = float2int(tempIndex2);
		F = 2.0f * interp(SineWaveform[tempIndex], SineWaveform[tempIndex+1], tempIndex2);
	};

	float CutOff;	//in Hz;
	float Q;		//0->1
	float F;
	twofloats Low, High, Band, Notch;
	filt_type Type;		//<0.33 = High, < 0.66 = Band, < 1 = Low

	bool FiltOn;
};

#endif
