<<<<<<< HEAD
//
// VST implementation of CSynth
//

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "synth.h"

// plugin definition

class CPlug : public AudioEffectX
{
protected:
	ERect rect;							// holds size of editor
	VstEvents* events;			// midi events
public:
	CSynth* synth;					// synth
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
BOOL APIENTRY DllMain( HINSTANCE  hModule, DWORD  reason, LPVOID lpReserved)
{ 
	hInstance = hModule; 
	return TRUE;
}
AEffect *main (audioMasterCallback audioMaster);
#endif

AEffect *main (audioMasterCallback audioMaster)
{ 
	CPlug* effect = new CPlug (audioMaster, 1, paCount);
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

	isSynth();
	hasVu ();
	canProcessReplacing ();
	cEffect.flags |= effFlagsHasEditor;	// has editor

	// create synth

	synth = new CSynth();
	
}

CPlug :: ~CPlug()
{

	// destroy synth

	delete synth;

}

// process

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
	CPoint p;

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

		// editor size

		case effEditGetRect: 
			if (synth->editor)
			{
				rect.left = 0;
				rect.top = 0;
				p = synth->editor->getSize();
				rect.right = p.x;
				rect.bottom = p.y;
			}
    	*(ERect**)ptr = &rect;
      result = 1;
    break;

		// open editor

=======
//
// VST implementation of CSynth
//

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "synth.h"

// plugin definition

class CPlug : public AudioEffectX
{
protected:
	ERect rect;							// holds size of editor
	VstEvents* events;			// midi events
public:
	CSynth* synth;					// synth
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
BOOL APIENTRY DllMain( HINSTANCE  hModule, DWORD  reason, LPVOID lpReserved)
{ 
	hInstance = hModule; 
	return TRUE;
}
AEffect *main (audioMasterCallback audioMaster);
#endif

AEffect *main (audioMasterCallback audioMaster)
{ 
	CPlug* effect = new CPlug (audioMaster, 1, paCount);
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

	isSynth();
	hasVu ();
	canProcessReplacing ();
	cEffect.flags |= effFlagsHasEditor;	// has editor

	// create synth

	synth = new CSynth();
	
}

CPlug :: ~CPlug()
{

	// destroy synth

	delete synth;

}

// process

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
	CPoint p;

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

		// editor size

		case effEditGetRect: 
			if (synth->editor)
			{
				rect.left = 0;
				rect.top = 0;
				p = synth->editor->getSize();
				rect.right = p.x;
				rect.bottom = p.y;
			}
    	*(ERect**)ptr = &rect;
      result = 1;
    break;

		// open editor

>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
		case effEditOpen:
#ifdef linux
			if (dp == 0)
				dp = (Display*)value;		
#endif
			if (synth->editor == 0) 	
			{
				synth->editor = new CSynthWin(synth);
<<<<<<< HEAD
				reparentWindow(synth->editor->getHandle(), (int)ptr);
				synth->editor->show();
			}
			break;

		// close editor

		case effEditClose:
			if (synth->editor)
			{
=======
				reparentWindow(synth->editor->getHandle(), (int)ptr);
				synth->editor->show();
			}
			break;

		// close editor

		case effEditClose:
			if (synth->editor)
			{
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
				delete synth->editor;
				synth->editor = 0;
			}
			break;
<<<<<<< HEAD
			
		// default

		default: 
			result = AudioEffect::dispatcher(opCode, index, value, ptr, opt);
	}
	
	return result;

}

// set param

void CPlug :: setParameter (long index, float value)
{
	synth->setParam(index, value); 
}

// get param

float CPlug :: getParameter (long index)
{
	return synth->getParam(index);
}

// get param name

void CPlug :: getParameterName (long index, char *text)
{
	switch (index)
	{
		case paCutoff: strcpy(text, "Cutoff"); break;
		case paQ: strcpy(text, "Q"); break;
	}
}

// get param value

void CPlug :: getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case paCutoff: sprintf(text, "%d", (int) (synth->getParam(index) * 100)); break;
		case paQ: sprintf(text, "%d", (int) (synth->getParam(index) * 100)); break;
	}
}

=======
			
		// default

		default: 
			result = AudioEffect::dispatcher(opCode, index, value, ptr, opt);
	}
	
	return result;

}

// set param

void CPlug :: setParameter (long index, float value)
{
	synth->setParam(index, value); 
}

// get param

float CPlug :: getParameter (long index)
{
	return synth->getParam(index);
}

// get param name

void CPlug :: getParameterName (long index, char *text)
{
	switch (index)
	{
		case paCutoff: strcpy(text, "Cutoff"); break;
		case paQ: strcpy(text, "Q"); break;
	}
}

// get param value

void CPlug :: getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case paCutoff: sprintf(text, "%d", (int) (synth->getParam(index) * 100)); break;
		case paQ: sprintf(text, "%d", (int) (synth->getParam(index) * 100)); break;
	}
}

>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
