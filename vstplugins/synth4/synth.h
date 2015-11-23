/*

synth.h
Copyright (C) 2007 jorgen www.linux-vst.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

*/

#ifndef __SYNTH__
#define __SYNTH__

#include "win.h"
#include "control.h"

const int VOLUMEENV = 0;
const int FILTERENV = 1;
const int NUMENVS = 2;
const int MAXENVTIME = 5;			// max 5 sek per stage
const int MAXVOICES = 8;			// polyphony

// params

const int paCutoff = 0;
const int paQ = 1;
const int paA1 = 2;
const int paD1 = 3;
const int paS1 = 4;
const int paR1 = 5;
const int paA2 = 6;
const int paD2 = 7;
const int paS2 = 8;
const int paR2 = 9;
const int paDetune = 10;
const int paPhase = 11;
const int paRingMod = 12;
const int paLFORate = 13;
const int paLFOVol = 14;
const int paLFOCutoff = 15;
const int paCount = 16;

const float PI = 3.1415f;
const float detuneScale = 0.5f;
const float phaseScale = 0.5f;
const float resoScale = 0.97f;

// envelope

const int ATTACK = 0;			// envelope stages
const int DECAY = 1;
const int SUSTAIN = 2;
const int RELEASE = 3;
const int ENVDONE = 4;

struct CEnvStage					// per voice
{
	bool running;						// envelope is running
	int stage;							// attack, decay, sustain or release
	float value, delta;			// current value and ramp
	int frames;							// frames left of this stage
};

class CEnv								// volume and filter envelope class
{
	protected:
		int maxFrames;				// maximum frames in a single envelope stage
	public:
		float attack, decay, sustain, release;
		CEnv(float a, float d, float s, float r)
			{ attack = a; decay  = d; sustain = s; release = r; }
		void setMaxFrames(int frames) 
			{ maxFrames = frames; }
		void reset(CEnvStage* p);
		void next(CEnvStage* p);
		void noteOff(CEnvStage* p);
};

void CEnv :: reset(CEnvStage* p)
{
	p->running = true;
	p->stage = ATTACK;		
	p->value = 0;
	p->frames = (int) (attack * attack * maxFrames);
	if (p->frames > 0)
		p->delta = 1.0f / (float)p->frames;
	else p->delta = 0;
}

void CEnv :: next(CEnvStage* p)
{
	if (p->stage == ATTACK)
	{
		p->stage = DECAY;
		p->value = 1;
		p->frames = (int) (decay * decay * maxFrames);
		if (p->frames > 0)
			p->delta = - (1 - sustain) / (float)p->frames;
		else p->delta = 0;
	}	
	else if (p->stage == DECAY)
	{
		p->running = false;	// stay at this stage until noteOff received
		p->stage = SUSTAIN;
		p->value = sustain;
		p->frames = 0;
		p->delta = 0;
	}
	else if (p->stage == RELEASE)
	{
		p->running = false;
		p->stage = ENVDONE;
		p->value = 0;
		p->frames = 0;
		p->delta = 0;
	}
}

void CEnv :: noteOff(CEnvStage* p)
{
	p->running = true;		
	p->stage = RELEASE;
	p->frames = (int) (release * release * maxFrames);
	if (p->frames < 128)	// prevent click with too fast release
	p->frames = 128;
	p->delta = - (p->value) / (float)p->frames;
}

// lfo

const int LFOSIZE = 1024;
const int LFOMIN = 1000;
const int LFOMAX = 20000;
class CLFO 
{
	public:
		float rate, vol, cutoff;				
		float data[LFOSIZE];
		CLFO();
};

CLFO :: CLFO()
{

	// lfo speed in ms
	
	rate = 0.5;
	
	// lfo depth
	
	vol = 0;
	cutoff = 0;

	// sinus look-up table

	int i, size = LFOSIZE;
	for (i = 0; i < LFOSIZE; i++)
		data[i] = 0.5 + 0.5 * sin(i / (float)LFOSIZE * 2 * PI);
		
}

// voice 

struct CVoice
{
	bool playing;								// true if voices is in use
	int note;										
	float freq1, phase1;					
	float freq2, phase2;					
	float low, band, high;			// filter
	CEnvStage env[NUMENVS];			// envelopes
	float lfo1, lfofreq1;
	float velVol, velCutoff;	// velocity to vol & cutoff
};

// polyphonic saw synth with filter and enevlopes

class CSynth
{
	protected:
		int rate;
		float volume, cutoff, reso;
		float detune, phase, ringMod;
		CVoice voices[MAXVOICES];
		CEnv* envelope[NUMENVS];
		CLFO* lfo;
	public:
		CWin* editor;
		CSynth();
		~CSynth();
		void midiInput(int data);
		void process(float* in1, float* in2, int samples);
		void setRate(int val);
		void allNotesOff();
		CVoice* needVoice();
		void setParam(int param, float value, bool automate = false);
		float getParam(int param);
};

// synth editor window

class CSynthWin : public CWin
{
	public: 
		CSynth* synth;
		CSynthWin(CSynth* plug);
};

// constructor

CSynth :: CSynth()
{
	
	// params

	volume = 0.5;
	cutoff = 0.5f;
	reso = 0;

	detune = 0.1f;
	phase = 0;
	ringMod = 0;
	
	// create envelopes

	envelope[0] = new CEnv(0, 0, 1, 0);
	envelope[1] = new CEnv(0, 0.1f, 0.25f, 1);
	
	// LFO
	
	lfo = new CLFO();

	// set default sample rate

	setRate(44100);

	// editor

	editor = 0;

	// reset all voices

	allNotesOff();

}

CSynth :: ~CSynth()
{

	// delete editor

	if (editor)
		delete editor;

	// free envelopes

	for (int i = 0; i < NUMENVS; i++)
		delete envelope[i];
		
	// free LFO
	
	delete lfo;

}

void CSynth :: setParam(int param, float value, bool automate)
{
	
	if (value < 0) 
		value = 0;
	else if (value > 1) 
		value = 1;

	switch (param)
	{
		case paRingMod: ringMod = value; break;
		case paDetune: detune = value; break;
		case paPhase: phase = value; break;
		case paCutoff: cutoff = value; break;
		case paQ: reso = value; break;
		case paA1: envelope[0]->attack = value; break;
		case paD1: envelope[0]->decay = value; break;
		case paS1: envelope[0]->sustain = value; break;
		case paR1: envelope[0]->release = value; break;
		case paA2: envelope[1]->attack = value; break;
		case paD2: envelope[1]->decay = value; break;
		case paS2: envelope[1]->sustain = value; break;
		case paR2: envelope[1]->release = value; break;
		case paLFORate: lfo->rate = value; break;
		case paLFOVol: lfo->vol = value; break;
		case paLFOCutoff: lfo->cutoff = value; break;
	}	

}

float CSynth :: getParam(int param)
{
	float result = 0;
	switch (param)
	{
		case paRingMod: result = ringMod; break;
		case paPhase: result = phase; break;
		case paDetune: result = detune; break;
		case paCutoff: result = cutoff; break;
		case paQ: result = reso; break;
		case paA1: result = envelope[0]->attack; break;
		case paD1: result = envelope[0]->decay; break;
		case paS1: result = envelope[0]->sustain; break;
		case paR1: result = envelope[0]->release; break;
		case paA2: result = envelope[1]->attack; break;
		case paD2: result = envelope[1]->decay; break;
		case paS2: result = envelope[1]->sustain; break;
		case paR2: result = envelope[1]->release; break;
		case paLFORate: result = lfo->rate; break;
		case paLFOVol: result = lfo->vol; break;
		case paLFOCutoff: result = lfo->cutoff; break;
	}	
	return result;
}

// set sample rate

void CSynth :: setRate(int val)
{

	// set internal sample rate

	rate = val;

	// update envelope max time

	for (int i = 0; i < NUMENVS; i++)
		envelope[i]->setMaxFrames(rate * MAXENVTIME);

}

// convert note to hertz

float noteToHz(float note)
{
	return (440 / (float)32) * (float)pow(2, ( (note - 9) / (float)12) );
}

// midi functions

int asStatus(int data)
{
	return data & 0xF0;
}

int asByte1(int data)
{
	return (data >> 8) & 0xFF;
}

int asByte2(int data)
{
	return (data >> 16) & 0xFF;
}

bool noteOn(int data)
{
	return asStatus(data) == 0x90 && asByte2(data) > 0;
}

bool noteOff(int data)
{
	return asStatus(data) == 0x80 || (asStatus(data) == 0x90 && asByte2(data) == 0);
}

// get avilable voice

CVoice* CSynth :: needVoice()
{
	CVoice* result = 0, *pv = voices;
	for (int i = 0; i < MAXVOICES; i++)
	{
		if (pv->playing == false)
		{
			result = pv;	// found free voice
			break;
		}
		pv++;		
	}	
	return result;
}

// handle midi input

void CSynth :: midiInput(int data)
{

	CVoice* pv;

	// note on

	if (noteOn(data))
	{
		pv = needVoice();
		if (pv)
		{ 
			pv->playing = true;

			// waveform

			pv->note = asByte1(data);

			pv->phase1 = 0;
			pv->phase2 = phase * phaseScale;

			pv->freq1 = 2 * noteToHz(pv->note - detune * detuneScale) / rate;
			pv->freq2 = 2 * noteToHz(pv->note + detune * detuneScale) / rate;

			// velocity
			
			pv->velVol = asMax(1, asByte2(data)) / (float)127;
			pv->velCutoff = 1;

			// filter

			pv->low = 0;
			pv->band = 0;
			pv->high = 0;

			// envelopes
			
			for (int i = 0; i < NUMENVS; i++)
				envelope[i]->reset(&pv->env[i]);

			// lfo
			
			pv->lfo1 = 0;
			pv->lfofreq1 = (LFOMIN + lfo->rate * (LFOMAX - LFOMIN)) * 
				0.001 * LFOSIZE / (float)rate;

		}
	}

	// note off

	else if (noteOff(data))
	{
		pv = voices;
		for (int i = 0; i < MAXVOICES; i++)
		{
			if (pv->playing && pv->note == asByte1(data) && pv->env[VOLUMEENV].stage != RELEASE)
			{
				for (int j = 0; j < NUMENVS; j++)				
					envelope[j]->noteOff(&pv->env[j]);
			}	
			pv++;		
		}	
	}

	// control change

	else if (asStatus(data) == 0xB0) 
	{
		// todo
	}

}

// all notes off

void CSynth :: allNotesOff()
{
	for (int i = 0; i < MAXVOICES; i++)
		voices[i].playing = false;
}

// synthesize

void CSynth :: process(float* in1, float* in2, int samples)
{

	int i;
	
	// per voice

	CVoice* pv = voices;
	for (i = 0; i < MAXVOICES; i++)
	{

		// render voice

		if (pv->playing)
		{

			int block, sub, j;
			float* p1 = in1;								// output buffers
			float* p2 = in2;

			block = samples;
			while (block > 0) 
			{

				sub = block;

				// process upto next envelope stage

				for (j = 0; j < NUMENVS; j++)
					if (pv->env[j].running && pv->env[j].frames < sub)
						sub = pv->env[j].frames;

				for (j = 0; j < sub; j++)
				{	

					// waveform

					float wave = ringMod * (pv->phase1 * pv->phase2) + 
						(1 - ringMod) * (pv->phase1 + pv->phase2);

					pv->phase1 = pv->phase1 + pv->freq1;
					if (pv->phase1 > 1)
						pv->phase1 -= 2;
					pv->phase2 = pv->phase2 + pv->freq2;
					if (pv->phase2 > 1)
						pv->phase2 -= 2;

					// filter

					float cut = 
						cutoff * 																												// main cutoff
						(pv->env[FILTERENV].value + j * pv->env[FILTERENV].delta) * 		// envelope
						(1 - lfo->cutoff * lfo->data[((int)(pv->lfo1)) & (LFOSIZE-1)]);	// lfo

    			pv->low = pv->low + cut * cut * pv->band;
    			pv->high = wave - pv->low - (1 - reso) * pv->band;
    			pv->band = pv->band + cut * cut * pv->high;
			
					// output

					float vol = 
						volume * 																											// main vol
						pv->velVol *																									// velovity 
						(pv->env[VOLUMEENV].value + j * pv->env[VOLUMEENV].delta) *		// env
						(1 - lfo->vol * lfo->data[((int)(pv->lfo1)) & (LFOSIZE-1)]);	// lfo
					
					*p1 = (float) (*p1++ + pv->low * vol);
					*p2 = (float) (*p2++ + pv->low * vol);
					
					pv->lfo1 += pv->lfofreq1;

				}

				block -= sub;

				// countdown envelopes

				for (j = 0; j < NUMENVS; j++)
				{
					if (pv->env[j].running)
					{
						pv->env[j].frames -= sub;
						pv->env[j].value += sub * pv->env[j].delta;
						if (pv->env[j].frames == 0)
							envelope[j]->next(&pv->env[j]);
						if (j == VOLUMEENV && pv->env[j].stage == ENVDONE)
						{
							pv->playing = false;	
							break;
						}
					}
				}

			}

		}
		
		// next voice

		pv++;

	}

}

// fader

class CFader : public CCtrl
{
	public:
		int pos, min, max;
		int mode;
		CSynth* synth;
		CFader();
		void paint(CGraphic *dc, CRect rc, int state);
		void mouseDown(int x, int y, int btn);
		void mouseMove(int x, int y, int btn);
		void setPos(int value);
		void changed();
};

CFader :: CFader()
	: CCtrl()
{
	pos = 0;
	min = 0;
	max = 100;
}

void CFader :: paint(CGraphic *dc, CRect rc, int state)
{

	// fader

	dc->setFillCol(asColor(64, 64, 0));
	dc->fillRect(asRect(rc.left, rc.top + 1, 
		rc.left + (int)((rc.right - rc.left) * pos / (float)max), rc.bottom - 1));

	// frame

	dc->setLineCol(asColor(0, 0, 0));
	dc->drawLine(rc.left + 1, rc.top, rc.right - 1, rc.top);
	dc->drawLine(rc.left, rc.top + 1, rc.left, rc.bottom - 1);
	dc->drawLine(rc.right - 1, rc.top + 1, rc.right - 1, rc.bottom - 1);
	dc->drawLine(rc.left + 1, rc.bottom - 1, rc.right - 1, rc.bottom - 1);

	// value

	dc->setTextCol(asColor(255, 255, 255));
	dc->drawText(asStr(pos), sizeRect(rc, 4, 0, 0, 0), alLeft);

}

void CFader :: mouseDown(int x, int y, int btn)
{
	if (btn & buLeft) 
		setPos(min + (int)(x / (float)width * (max - min)));
}

void CFader :: mouseMove(int x, int y, int btn)
{
	if (btn & buLeft) 
		setPos(min + (int)(x / (float)width * (max - min)));
}

void CFader :: setPos(int value)
{
	int temp;
	if (value < min) 
		temp = min;
	else if (value > max)
		temp = max;
	else temp = value;
	if (temp != pos)
	{
		pos = temp;
		redraw();
		changed();
	}
}

void CFader :: changed()
{
	synth->setParam(mode, (pos - min) / (float)(max - min), true);
}

void addFader(CWin* win, CSynth* synth, int x, int & y, int mode, CStr name)
{
	CLabel* la = new CLabel();
	la->setBounds(x, y, 128, 16);
	la->text = name;
	win->addControl(la);
	y += 20;

	CFader* co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = mode;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	win->addControl(co);
	y += 20;
}

// editor

CSynthWin :: CSynthWin(CSynth* plug)
 : CWin()
{

	synth = plug;

	// add controls

	int x = 16, y = 16;
	CFader* co;
	CLabel* la;
	
	// detune	
	addFader(this, synth, x, y, paDetune, "Detune");
	
	// phase
	
	addFader(this, synth, x, y, paPhase, "Phase");

	// ringMod
	
	addFader(this, synth, x, y, paRingMod, "Ring mod.");

	// volume ADSR

	la = new CLabel();
	la->setBounds(x, y, 128, 16);
	la->text = "ADSR";
	addControl(la);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paA1;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paD1;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paS1;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paR1;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	// LFO rate
	
	addFader(this, synth, x, y, paLFORate, "LFO rate");

	y = 16;
	x += 128 + 32;

	// cutoff
	
	addFader(this, synth, x, y, paCutoff, "Cutoff");
	
	// q

	addFader(this, synth, x, y, paQ, "Q");

	// filter ADSR

	la = new CLabel();
	la->setBounds(x, y, 128, 16);
	la->text = "ADSR";
	addControl(la);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paA2;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paD2;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paS2;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	co = new CFader();
	co->setBounds(x, y, 128, 16);
	co->synth = synth;
	co->mode = paR2;
	co->pos = (int)(synth->getParam(co->mode) * co->max);
	addControl(co);
	y += 20;

	// LFO -> vol
	
	addFader(this, synth, x, y, paLFOVol, "LFO to volume");
	
	// LFO -> cutoff
		
	addFader(this, synth, x, y, paLFOCutoff, "LFO to cutoff");
	
	// set size of window

	setSize(320, 284);

}

#endif
