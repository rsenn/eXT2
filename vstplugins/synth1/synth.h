<<<<<<< HEAD
#ifndef __SYNTH__
#define __SYNTH__

// simple synth class

class CSynth
{
	protected:
		int rate;
		double vol;
		double freq, phase;	
		bool playing;
	public:
		CSynth();
		void midiInput(int data);
		void process(float* p1, float* p2, int samples);
		void setRate(int val) { rate = val; }
};

CSynth :: CSynth()
{
	vol = 0.25;
	playing = false;
	rate = 44100;
}

// convert note to hertz

double noteToHz(int note)
{
	return (440 / (float)32) * pow(2, ( (note - 9) / (float)12) );
}

// handle midi input

void CSynth :: midiInput(int data)
{

	unsigned char status = data & 0xF0;
	unsigned char byte1 = (data >> 8) & 0xFF;
	unsigned char byte2 = (data >> 16) & 0xFF;

	// note on

	if (status == 0x90 && byte2 > 0) 
	{
		phase = 0;
		freq = 2 * noteToHz(byte1) / rate;
		playing = true;
	}

	// note off

	else if (status == 0x80 || (status == 0x90 && byte2 == 0)) 
	{
		playing = false;
	}

	// control change

	else if (status == 0xB0) 
	{
		// todo
	}

}

// synthesize

void CSynth :: process(float* p1, float* p2, int samples)
{

	int i;

	if (playing)
	{

		for (i = 0; i < samples; i++)
		{	

			*p1 = (float) (*p1 + phase * vol);
			*p2 = (float) (*p2 + phase * vol);

			p1++;
			p2++;

			phase = phase + freq;
			if (phase > 1)
				phase -= 2;

		}

	}

}

=======
#ifndef __SYNTH__
#define __SYNTH__

// simple synth class

class CSynth
{
	protected:
		int rate;
		double vol;
		double freq, phase;	
		bool playing;
	public:
		CSynth();
		void midiInput(int data);
		void process(float* p1, float* p2, int samples);
		void setRate(int val) { rate = val; }
};

CSynth :: CSynth()
{
	vol = 0.25;
	playing = false;
	rate = 44100;
}

// convert note to hertz

double noteToHz(int note)
{
	return (440 / (float)32) * pow(2, ( (note - 9) / (float)12) );
}

// handle midi input

void CSynth :: midiInput(int data)
{

	unsigned char status = data & 0xF0;
	unsigned char byte1 = (data >> 8) & 0xFF;
	unsigned char byte2 = (data >> 16) & 0xFF;

	// note on

	if (status == 0x90 && byte2 > 0) 
	{
		phase = 0;
		freq = 2 * noteToHz(byte1) / rate;
		playing = true;
	}

	// note off

	else if (status == 0x80 || (status == 0x90 && byte2 == 0)) 
	{
		playing = false;
	}

	// control change

	else if (status == 0xB0) 
	{
		// todo
	}

}

// synthesize

void CSynth :: process(float* p1, float* p2, int samples)
{

	int i;

	if (playing)
	{

		for (i = 0; i < samples; i++)
		{	

			*p1 = (float) (*p1 + phase * vol);
			*p2 = (float) (*p2 + phase * vol);

			p1++;
			p2++;

			phase = phase + freq;
			if (phase > 1)
				phase -= 2;

		}

	}

}

>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
#endif
