// VST header/source files
#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include <iostream>
using namespace std;

#include "synth.h"

// plugin definition

class CPlug : public AudioEffectX
{
public:

	CSynth* synth;					// synth
	VstEvents* events;			// midi events

	CPlug (audioMasterCallback audioMaster, long numPrograms, long numParams);
	~CPlug ();
	void process (float **inputs, float **outputs, long sampleFrames);
	void processReplacing (float **inputs, float **outputs, long sampleFrames);
	long dispatcher(long opCode, long index, long value, void *ptr, float opt);

};

// main function

#ifdef __GNUC__ 
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin
#else
AEffect *main (audioMasterCallback audioMaster);
#endif

AEffect *main (audioMasterCallback audioMaster)
{ 
	CPlug* effect = new CPlug (audioMaster, 1, 16);
	if (!effect)
		return 0;
	return effect->getAeffect ();
}

// plugin implementation

CPlug :: CPlug(audioMasterCallback audioMaster, long numPrograms, long numParams)
	: AudioEffectX(audioMaster, numPrograms, numParams)
{

	// plugin id

	setUniqueID ((int)'nerd');

	// stereo output

	setNumInputs (0);	
	setNumOutputs (2);	

	hasVu ();
	canProcessReplacing ();

	// create synth

	synth = new CSynth();

}

CPlug :: ~CPlug()
{

	// destroy synth

	delete synth;

}

void CPlug :: process (float **inputs, float **outputs, long sampleFrames)
{
	
	int i, cue, block;
	VstMidiEvent* e;
	
	// outputs buffers

	float* p1 = outputs[0];
	float* p2 = outputs[1];

	// process audio on midi events

	if (events) 
	{
        cout << "midi processing" << events->numEvents << endl;

		cue = 0;
		for (i = 0; i < events->numEvents; i++)
		{
			e = (VstMidiEvent*) events->events[i];
			if (e->type == kVstMidiType)
			{
				block = e->deltaFrames - cue;
				if (block > 0) 
				{
					synth->process(p1, p2, block);
					p1 += block;
					p2 += block;
				}				

                cout << "do processing " << i << endl;

				synth->midiInput(
					e->midiData[0] + (e->midiData[1] << 8) + (e->midiData[2] << 16) );
				cue = e->deltaFrames;

                cout << "end processing " << i << endl;
			}
		}

	}

    cout << "pre processing" << endl;

	synth->process(p1, p2, sampleFrames - cue);

    cout << "post processing" << endl;
	// release events pointer

	events = 0;
	
}

void CPlug :: processReplacing (float **inputs, float **outputs, long sampleFrames)
{
	process(inputs, outputs, sampleFrames);
}

long CPlug :: dispatcher (long opCode, long index, long value, void *ptr, float opt)
{
	int result = 0;

	switch (opCode) 
	{

		// set sample rate
		
    case effSetSampleRate:
			synth->setRate((int)opt);
			break;

		// process events

		case effProcessEvents: 
      events = (VstEvents*)ptr;
		cout << "events: " << events->numEvents << endl;
      result = 1;
    break;

		// default 

		default: 
			result = AudioEffect::dispatcher(opCode, index, value, ptr, opt);
	}
	
	return result;

}

