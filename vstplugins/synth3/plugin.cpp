<<<<<<< HEAD
//
// VST implementation of CSynth
//

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "synth.h"

// params

const int CUTOFF = 0;
const int RESO = 1;
const int VOLA = 2;
const int VOLD = 3;
const int VOLS = 4;
const int VOLR = 5;
const int FILTERA = 6;
const int FILTERD = 7;
const int FILTERS = 8;
const int FILTERR = 9;

const int PARAMS = 10;

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
	void setParameter (long index, float value);
	float getParameter (long index);
	void getParameterName (long index, char *text);
	void getParameterDisplay (long index, char *text);
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
	CPlug* effect = new CPlug (audioMaster, 1, PARAMS);
	if (!effect)
		return 0;
	return effect->getAeffect ();
}

// plugin implementation

CPlug :: CPlug(audioMasterCallback audioMaster, long numPrograms, long numParams)
	: AudioEffectX(audioMaster, numPrograms, numParams)
{

	// plugin id

	setUniqueID ((int)"test");

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
				synth->midiInput(
					e->midiData[0] + (e->midiData[1] << 8) + (e->midiData[2] << 16) );
				cue = e->deltaFrames;
			}
		}

	}

	synth->process(p1, p2, sampleFrames - cue);

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
      result = 1;
    break;

		// default 

		default: 
			result = AudioEffect::dispatcher(opCode, index, value, ptr, opt);
	}
	
	return result;

}

// set param

void CPlug :: setParameter (long index, float value)
{
	switch (index)
	{
		case CUTOFF: synth->setCutoff(value); break;
		case RESO: synth->setReso(value); break;
		case VOLA: synth->setVolA(value); break;
		case VOLD: synth->setVolD(value); break;
		case VOLS: synth->setVolS(value); break;
		case VOLR: synth->setVolR(value); break;
		case FILTERA: synth->setFilterA(value); break;
		case FILTERD: synth->setFilterD(value); break;
		case FILTERS: synth->setFilterS(value); break;
		case FILTERR: synth->setFilterR(value); break;
	}
}

// get param

float CPlug :: getParameter (long index)
{
	float result = 0;
	switch (index)
	{
		case CUTOFF: result = (float) synth->getCutoff(); break;
		case RESO: result = (float) synth->getReso(); break;
		case VOLA: result = (float) synth->getVolA(); break;
		case VOLD: result = (float) synth->getVolD(); break;
		case VOLS: result = (float) synth->getVolS(); break;
		case VOLR: result = (float) synth->getVolR(); break;
		case FILTERA: result = (float) synth->getFilterA(); break;
		case FILTERD: result = (float) synth->getFilterD(); break;
		case FILTERS: result = (float) synth->getFilterS(); break;
		case FILTERR: result = (float) synth->getFilterR(); break;
	}
	return result;
}

// get param name

void CPlug :: getParameterName (long index, char *text)
{
	switch (index)
	{
		case CUTOFF: strcpy(text, "Cutoff"); break;
		case RESO: strcpy(text, "Q"); break;
		case VOLA: strcpy(text, "Vol A"); break;
		case VOLD: strcpy(text, "Vol D"); break;
		case VOLS: strcpy(text, "Vol S"); break;
		case VOLR: strcpy(text, "Vol R"); break;
		case FILTERA: strcpy(text, "Filter A"); break;
		case FILTERD: strcpy(text, "Filter D"); break;
		case FILTERS: strcpy(text, "Filter S"); break;
		case FILTERR: strcpy(text, "Filter R"); break;
	}
}

// get param value

void CPlug :: getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case CUTOFF: sprintf(text, "%d", (int) (synth->getCutoff() * 100)); break;
		case RESO: sprintf(text, "%d", (int) (synth->getReso() * 100)); break;
		case VOLA: sprintf(text, "%d", (int) (synth->getVolA() * 100)); break;
		case VOLD: sprintf(text, "%d", (int) (synth->getVolD() * 100)); break;
		case VOLS: sprintf(text, "%d", (int) (synth->getVolS() * 100)); break;
		case VOLR: sprintf(text, "%d", (int) (synth->getVolR() * 100)); break;
		case FILTERA: sprintf(text, "%d", (int) (synth->getFilterA() * 100)); break;
		case FILTERD: sprintf(text, "%d", (int) (synth->getFilterD() * 100)); break;
		case FILTERS: sprintf(text, "%d", (int) (synth->getFilterS() * 100)); break;
		case FILTERR: sprintf(text, "%d", (int) (synth->getFilterR() * 100)); break;
	}
}

=======
//
// VST implementation of CSynth
//

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "synth.h"

// params

const int CUTOFF = 0;
const int RESO = 1;
const int VOLA = 2;
const int VOLD = 3;
const int VOLS = 4;
const int VOLR = 5;
const int FILTERA = 6;
const int FILTERD = 7;
const int FILTERS = 8;
const int FILTERR = 9;

const int PARAMS = 10;

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
	void setParameter (long index, float value);
	float getParameter (long index);
	void getParameterName (long index, char *text);
	void getParameterDisplay (long index, char *text);
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
	CPlug* effect = new CPlug (audioMaster, 1, PARAMS);
	if (!effect)
		return 0;
	return effect->getAeffect ();
}

// plugin implementation

CPlug :: CPlug(audioMasterCallback audioMaster, long numPrograms, long numParams)
	: AudioEffectX(audioMaster, numPrograms, numParams)
{

	// plugin id

	setUniqueID ((int)"test");

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
				synth->midiInput(
					e->midiData[0] + (e->midiData[1] << 8) + (e->midiData[2] << 16) );
				cue = e->deltaFrames;
			}
		}

	}

	synth->process(p1, p2, sampleFrames - cue);

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
      result = 1;
    break;

		// default 

		default: 
			result = AudioEffect::dispatcher(opCode, index, value, ptr, opt);
	}
	
	return result;

}

// set param

void CPlug :: setParameter (long index, float value)
{
	switch (index)
	{
		case CUTOFF: synth->setCutoff(value); break;
		case RESO: synth->setReso(value); break;
		case VOLA: synth->setVolA(value); break;
		case VOLD: synth->setVolD(value); break;
		case VOLS: synth->setVolS(value); break;
		case VOLR: synth->setVolR(value); break;
		case FILTERA: synth->setFilterA(value); break;
		case FILTERD: synth->setFilterD(value); break;
		case FILTERS: synth->setFilterS(value); break;
		case FILTERR: synth->setFilterR(value); break;
	}
}

// get param

float CPlug :: getParameter (long index)
{
	float result = 0;
	switch (index)
	{
		case CUTOFF: result = (float) synth->getCutoff(); break;
		case RESO: result = (float) synth->getReso(); break;
		case VOLA: result = (float) synth->getVolA(); break;
		case VOLD: result = (float) synth->getVolD(); break;
		case VOLS: result = (float) synth->getVolS(); break;
		case VOLR: result = (float) synth->getVolR(); break;
		case FILTERA: result = (float) synth->getFilterA(); break;
		case FILTERD: result = (float) synth->getFilterD(); break;
		case FILTERS: result = (float) synth->getFilterS(); break;
		case FILTERR: result = (float) synth->getFilterR(); break;
	}
	return result;
}

// get param name

void CPlug :: getParameterName (long index, char *text)
{
	switch (index)
	{
		case CUTOFF: strcpy(text, "Cutoff"); break;
		case RESO: strcpy(text, "Q"); break;
		case VOLA: strcpy(text, "Vol A"); break;
		case VOLD: strcpy(text, "Vol D"); break;
		case VOLS: strcpy(text, "Vol S"); break;
		case VOLR: strcpy(text, "Vol R"); break;
		case FILTERA: strcpy(text, "Filter A"); break;
		case FILTERD: strcpy(text, "Filter D"); break;
		case FILTERS: strcpy(text, "Filter S"); break;
		case FILTERR: strcpy(text, "Filter R"); break;
	}
}

// get param value

void CPlug :: getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case CUTOFF: sprintf(text, "%d", (int) (synth->getCutoff() * 100)); break;
		case RESO: sprintf(text, "%d", (int) (synth->getReso() * 100)); break;
		case VOLA: sprintf(text, "%d", (int) (synth->getVolA() * 100)); break;
		case VOLD: sprintf(text, "%d", (int) (synth->getVolD() * 100)); break;
		case VOLS: sprintf(text, "%d", (int) (synth->getVolS() * 100)); break;
		case VOLR: sprintf(text, "%d", (int) (synth->getVolR() * 100)); break;
		case FILTERA: sprintf(text, "%d", (int) (synth->getFilterA() * 100)); break;
		case FILTERD: sprintf(text, "%d", (int) (synth->getFilterD() * 100)); break;
		case FILTERS: sprintf(text, "%d", (int) (synth->getFilterS() * 100)); break;
		case FILTERR: sprintf(text, "%d", (int) (synth->getFilterR() * 100)); break;
	}
}

>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
