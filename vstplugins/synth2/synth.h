#ifndef __SYNTH__
#define __SYNTH__

// simple synth with filter

class CSynth
{
	protected:
		int rate;
		double vol, cutoff, reso;
		double freq, phase;	
		bool playing;
		double low, band, high;
	public:
		CSynth();
		void midiInput(int data);
		void process(float* p1, float* p2, int samples);
		void setRate(int val) { rate = val; }
		void setCutoff(double val) { cutoff = val; }
		void setReso(double val) { reso = val; }
		double getCutoff() { return cutoff; }
		double getReso() { return reso; }
};

// constructor

CSynth :: CSynth()
{
	setRate(44100);
	vol = 0.25;
	cutoff = 0.2;
	reso = 0.5;
	playing = false;
}

// convert note to hertz

double noteToHz(int note)
{
	return (440 / (float)32) * pow(2, ( (note - 9) / (float)12) );
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

// handle midi input

void CSynth :: midiInput(int data)
{

	// note on

	if (noteOn(data)) 
	{
		phase = 0;
		freq = 2 * noteToHz(asByte1(data)) / rate;
		low = 0;
		band = 0;
		high = 0;
		playing = true;
	}

	// note off

	else if (noteOff(data)) 
	{
		playing = false;
	}

	// control change

	else if (asStatus(data) == 0xB0) 
	{
		// todo
	}

}

// synthesize

void CSynth :: process(float* p1, float* p2, int samples)
{

	int i;
	double val, cut;

	if (playing)
	{

		cut = cutoff * cutoff;	// smoother range

		for (i = 0; i < samples; i++)
		{	

			// waveform

			val = phase;
			phase = phase + freq;
			if (phase > 1)
				phase -= 2;

			// filter

    	low = low + cut * band;
    	high = val - low - (1 - reso) * band;
    	band = band + cut * high;
			
			// output

			*p1 = (float) (*p1++ + low * vol);	// using lowpass
			*p2 = (float) (*p2++ + low * vol);

		}

	}

}

#endif
