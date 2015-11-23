#ifndef __SYNTH__
#define __SYNTH__

const int VOLUMEENV = 0;
const int FILTERENV = 1;
const int NUMENVS = 2;
const int MAXENVTIME = 5;	// max 5 sek per stage
const int MAXVOICES = 8;	

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

// voice 

struct CVoice
{
	bool playing;								// true if voices is in use
	int note;										
	float freq, phase;					// waveform
	float low, band, high;			// filter
	CEnvStage env[NUMENVS];			// envelopes
};

// polyphonic saw synth with filter and enevlopes

class CSynth
{
	protected:
		int rate;
		float volume, cutoff, reso;
		CVoice voices[MAXVOICES];
		CEnv* envelope[NUMENVS];
	public:
		CSynth();
		~CSynth();
		void midiInput(int data);
		void process(float* in1, float* in2, int samples);
		void setRate(int val);
		void allNotesOff();
		CVoice* needVoice();
		void setCutoff(float val) { cutoff = val; }
		void setReso(float val) { reso = val; }
		float getCutoff() { return cutoff; }
		float getReso() { return reso; }
		void setVolA(float val) { envelope[0]->attack = val; }
		void setVolD(float val) { envelope[0]->decay = val; }
		void setVolS(float val) { envelope[0]->sustain = val; }
		void setVolR(float val) { envelope[0]->release = val; }
		float getVolA() { return envelope[0]->attack; }
		float getVolD() { return envelope[0]->decay; }
		float getVolS() { return envelope[0]->sustain; }
		float getVolR() { return envelope[0]->release; }
		void setFilterA(float val) { envelope[1]->attack = val; }
		void setFilterD(float val) { envelope[1]->decay = val; }
		void setFilterS(float val) { envelope[1]->sustain = val; }
		void setFilterR(float val) { envelope[1]->release = val; }
		float getFilterA() { return envelope[1]->attack; }
		float getFilterD() { return envelope[1]->decay; }
		float getFilterS() { return envelope[1]->sustain; }
		float getFilterR() { return envelope[1]->release; }
};

// constructor

CSynth :: CSynth()
{
	
	// parames

	volume = 0.5;
	cutoff = 0.75f;
	reso = 0.5;
	
	// create envelopes

	envelope[0] = new CEnv(0, 0, 1, 0);
	envelope[1] = new CEnv(0, 0.1f, 0.25f, 1);

	// set default sample rate

	setRate(44100);

	// reset all voices

	allNotesOff();

}

CSynth :: ~CSynth()
{

	// free envelopes

	for (int i = 0; i < NUMENVS; i++)
		delete envelope[i];	

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

float noteToHz(int note)
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

			pv->phase = 0;
			pv->note = asByte1(data);
			pv->freq = 2 * noteToHz(pv->note) / rate;

			// filter

			pv->low = 0;
			pv->band = 0;
			pv->high = 0;

			// envelope
			
			for (int i = 0; i < NUMENVS; i++)
				envelope[i]->reset(&pv->env[i]);

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

					float val = pv->phase;
					pv->phase = pv->phase + pv->freq;
					if (pv->phase > 1)
						pv->phase -= 2;

					// filter

					float cut = cutoff * (pv->env[FILTERENV].value + j * pv->env[FILTERENV].delta);

    			pv->low = pv->low + cut * cut * pv->band;
    			pv->high = val - pv->low - (1 - reso) * pv->band;
    			pv->band = pv->band + cut * cut * pv->high;
			
					// output

					float vol = volume * (pv->env[VOLUMEENV].value + j * pv->env[VOLUMEENV].delta);

					*p1 = (float) (*p1++ + pv->low * vol * vol);
					*p2 = (float) (*p2++ + pv->low * vol * vol);

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

#endif
