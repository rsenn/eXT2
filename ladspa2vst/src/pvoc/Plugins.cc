/*
	Plugins.cc
	
	Copyright 2004 Tim Goetze <tim@quitte.de>
	Copyright 2001-2 Richard Dobson, Trevor Wishart

	http://quitte.de/dsp/

	phase vocoder plugins designed by Richard Dobson and Trevor Wishart,
	LADSPA adaptation by Tim Goetze

*/
/* 
	The original file (plugins.c) states:
 
	License Pending. This code is Copyright (c) Trevor Wishart, Richard Dobson 
	and Composers Desktop Project, March 2001. This code is made freely available 
	for non-commercial use. This copyright notice should be preserved in any 
	redistributed and/or modified version.
 */
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA or point your web browser to http://www.gnu.org.
*/

#include "basics.h"

#include "Plugins.h"
#include "Descriptor.h"

void
PvocPlugin::init (double _fs)
{
	fs = _fs;
	
	in.pvoc.init ((int) _fs, FFTLEN, DECFAC, PVPP_STREAMING);
	out.pvoc.init ((int) _fs, FFTLEN, DECFAC, PVPP_STREAMING);
}

void
PvocPlugin::activate()
{
	in.fill = 0;
	
	memset (in.buffer, 0, sizeof (in.buffer));
	memset (out.buffer, 0, sizeof (out.buffer));

	memset (frame, 0, sizeof (frame));
}

/* //////////////////////////////////////////////////////////////////////// */

/******* EXAGGERATE **********/


int pv_normalise(float *sbufptr,double pre_totalamp,double post_totalamp,int wanted)
{
	double normaliser;
	int vc,zeroset = 0;
	if(post_totalamp < NOISE_FLOOR)	
		zeroset = 1;		
 	else {
		/*normaliser = pre_totalamp/post_totalamp;*/
		/*RWD, needs a bit more trimming ! */
		/*tg, trimming seems fine */
		normaliser = (pre_totalamp/post_totalamp) * 0.5;	

		for(vc = 0; vc < wanted; vc += 2)
			sbufptr[vc] = (float)(sbufptr[vc] * normaliser);
	}
	return zeroset;
}

/*NB clength will always be wanted/2 */
int pv_specexag(int clength,float *sbufptr,double exag_exag,int wanted)				 
{
	int cc, vc;	
	double post_totalamp = 0.0, pre_totalamp = 0.0;
	double maxamp = 0.0, normaliser;

	for( cc = 0 ,vc = 0; cc < clength; cc++, vc += 2)  {
		pre_totalamp += sbufptr[vc];
		if(sbufptr[vc] > maxamp)
			maxamp = sbufptr[vc];
		
	}
	if(maxamp<=0.0)
		return 1;		/*pseudo error: announce a zero*/
	normaliser = 1.0/maxamp;
	for(cc = 0 ,vc = 0; cc < clength; cc++, vc += 2)  {
		sbufptr[vc]  = (float)(sbufptr[vc] * normaliser);
		sbufptr[vc]  = (float)(pow(sbufptr[vc],exag_exag));
		post_totalamp += sbufptr[vc];
	}
	/*RWD TODO: do pre/post calc here, so normalize() is a simple vector op */
	/*tg, doesn't seem to save us anything to do so */
	return pv_normalise(sbufptr,pre_totalamp,post_totalamp,wanted);
}

/* //////////////////////////////////////////////////////////////////////// */

void
Exaggerate::init (double _fs)
{
	PvocPlugin::init (_fs);
}

void
Exaggerate::activate()
{
	PvocPlugin::activate();
}

template <sample_func_t F>
void
Exaggerate::one_cycle (int frames)
{
	d_sample * s = ports[0];
	d_sample amount = *ports[1];
	d_sample * d = ports[2];

	while (frames)
	{
		int n = min (frames, DECFAC - in.fill);

		for (int i = 0; i < n; ++i)
		{
			in.buffer[i + in.fill] = s[i];
			F (d, i, out.buffer[in.fill + i], adding_gain);
		}

		in.fill += n;
		s += n;
		d += n;
		
		if (in.fill == DECFAC)
		{
			in.pvoc.generate_frame (in.buffer, frame, DECFAC, PVOC_AMP_FREQ);
			pv_specexag (NBINS, frame, amount, NBINS * 2);
			out.pvoc.process_frame (frame, out.buffer, PVOC_AMP_FREQ);
			in.fill = 0;
		}

		frames -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Exaggerate::port_info [] =
{
	{
		"in",
		INPUT | AUDIO,
		{0}
	}, {
		"amount",
		INPUT | CONTROL,
		{BOUNDED | DEFAULT_1, -1, 10}
	}, {
		"out",
		OUTPUT | AUDIO,
		{0}
	}
};

template <>
Descriptor<Exaggerate>::Descriptor()
{
	UniqueID = 1791;
	Label = "Exaggerate";
	Properties = HARD_RT;

	Name = "spectral exaggerator";
	Maker = "Richard Dobson, Trevor Wishart, Tim Goetze <tim@quitte.de>";
	Copyright = "GPL, 1981-2004";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

/* split/merge amp and freq in frames */

void get_amp_and_frq(const float *floatbuf,float *amp,float *freq,long clength)
{
	long cc, vc;

	for( cc = 0 ,vc = 0; cc < clength; cc++, vc += 2){
		amp[cc]  = floatbuf[vc];
		freq[cc] = floatbuf[vc+1];
	}
	
}

void put_amp_and_frq(float *floatbuf,const float *amp, const float *freq,long clength)
{
	long cc, vc;

	for(cc = 0, vc = 0; cc < clength; cc++, vc += 2){
		floatbuf[vc] = amp[cc];
		floatbuf[vc+1] = freq[cc];
	}

}

/*RWD: NB: calcs the ~theoretical~ chan-no...doesn't always correspond to reality! */
int get_channel_corresponding_to_frq(double thisfrq,double chwidth,double nyquist)
{	
	int chan;
	double halfchwidth = chwidth * 0.5;

 	chan = (int)((fabs(thisfrq) + halfchwidth)/chwidth);  /* TRUNCATE */

	return chan;
}

/******** SPEC TRANSP *************/
#define OCTAVES_PER_SEMITONE (0.08333333333)

float convert_shiftp_vals(float semitone)
{
	float val;	
	val = (float)(semitone * OCTAVES_PER_SEMITONE);
	val = (float) pow(2.0,val);
	return val;
}


void do_spectral_shiftp(float *amp, float *freq,float pitch,long clength)
{
	double shft = (double) pitch;
	long   j, k;
	
	if( shft > 1.0f) {
		j = clength-1;
		k  = lrint((double)j/shft);
		while( k >= 0) {
			/*RWD*/
			if(j < 0)
				break;
			amp[j]  = amp[k];
			freq[j] = (float)(shft * freq[k]);
			j-- ;
			k  = lrint((double)j/shft);
		}
		for( k=j; k>= 0;k-- ) {  /*RWD was k > */
			amp[k]  = 0.0f;
			freq[k] = 0.0f;
		}				
	} else if(shft < 1.0){		/*RWD : shft = 1 = no change */
		j = 0;
		k  = lrint((double)j/shft);
		while( k <= (clength-1)) {
			amp[j]  = amp[k];
			freq[j] = (float)(shft * freq[k]);
			j++ ;
			k  = lrint((double)j/shft);
		}
		for( k=j; k < clength; k++ ) {
			amp[k]  = 0.0f;
			freq[k] = 0.0f;
		}				
	}
}

void
Transpose::init (double _fs)
{
	PvocPlugin::init (_fs);
}

void
Transpose::activate()
{
	PvocPlugin::activate();
}

template <sample_func_t F>
void
Transpose::one_cycle (int frames)
{
	d_sample * s = ports[0];
	d_sample amount = *ports[1];
	d_sample * d = ports[2];

	while (frames)
	{
		int n = min (frames, DECFAC - in.fill);

		for (int i = 0; i < n; ++i)
		{
			in.buffer[i + in.fill] = s[i];
			F (d, i, out.buffer[in.fill + i], adding_gain);
		}

		in.fill += n;
		s += n;
		d += n;
		
		if (in.fill == DECFAC)
		{
			in.pvoc.generate_frame (in.buffer, frame, DECFAC, PVOC_AMP_FREQ);
			get_amp_and_frq (frame, amp, freq, NBINS);
			do_spectral_shiftp (amp, freq, convert_shiftp_vals (amount), NBINS);
			put_amp_and_frq (frame, amp, freq, NBINS);
			out.pvoc.process_frame (frame, out.buffer, PVOC_AMP_FREQ);
			in.fill = 0;
		}

		frames -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Transpose::port_info [] =
{
	{
		"in",
		INPUT | AUDIO,
		{0}
	}, {
		"transpose",
		INPUT | CONTROL,
		{BOUNDED | DEFAULT_0, -24, 24}
	}, {
		"out",
		OUTPUT | AUDIO,
		{0}
	}
};

template <>
Descriptor<Transpose>::Descriptor()
{
	UniqueID = 1792;
	Label = "Transpose";
	Properties = HARD_RT;

	Name = "phase-vocoder based pitch shifter";
	Maker = "Richard Dobson, Trevor Wishart, Tim Goetze <tim@quitte.de>";
	Copyright = "GPL, 1981-2004";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void pv_accumulate(int index,float *sbufptr, float *sampbuf)
{
	
	int frq = index+1;
	if(sbufptr[index] > sampbuf[index])  {	 /* if current amp > amp in accumulator */
		sampbuf[index] = sbufptr[index]; 
		sampbuf[frq] = sbufptr[frq];	 /* replace amp in accumulator with current amp */
	} else {
		sbufptr[index] = sampbuf[index]; 	 /* else replace current amp with amp in accumulator */
		sbufptr[frq] = sampbuf[frq];								
	}	
}

void pv_specaccu(char glisflag,
				 char decayflag,
				 double dindex, double gindex,
				 int clength,
				 float *sampbuf_0, 		
				 float *sbufptr_0,float nyquist)
{
	
	int vc, cc;
	if(glisflag) {
		if(decayflag) {		 /*ie both set*/
			for(cc = 0, vc = 0; cc< clength; cc++,  vc += 2) {
				sampbuf_0[vc] = (float)(sampbuf_0[vc] * dindex);
				sampbuf_0[vc+1] = (float)(sampbuf_0[vc+1] * gindex);
				/*RWD avoid nyquist overruns?*/
				if(sampbuf_0[vc+1] >= nyquist)
					sampbuf_0[vc] = 0.0f;
				pv_accumulate(vc,sbufptr_0,sampbuf_0);				
			}
		} else {					   /*just GLIS*/
			for(cc = 0, vc = 0; cc < clength; cc++,  vc += 2) {
				sampbuf_0[vc+1] = (float)(sampbuf_0[vc+1] * gindex);
				/*RWD avoid nyquist overruns?*/
				if(sampbuf_0[vc+1] >= nyquist)
					sampbuf_0[vc] = 0.0f;
				pv_accumulate(vc,sbufptr_0,sampbuf_0);				
			}
		}
	} else if(decayflag) {	  /*ie only this set*/
		for(cc = 0, vc = 0; cc < clength; cc++,  vc += 2) {
			sampbuf_0[vc] = (float)(sampbuf_0[vc] * dindex);			
			pv_accumulate(vc,sbufptr_0,sampbuf_0);			
		}
	} else {	 						 /*neither*/
		for(cc = 0, vc = 0; cc < clength; cc++,  vc += 2) {			
			pv_accumulate(vc,sbufptr_0,sampbuf_0);			
		}
	}
	
}


void
Accumulate::init (double _fs)
{
	PvocPlugin::init (_fs);

	nyquist = fs * .45;
	arate = fs / (float) DECFAC;
	frametime = 1 / arate;
}

void
Accumulate::activate()
{
	PvocPlugin::activate();

	memset (framestore, 0, sizeof (framestore));
}

template <sample_func_t F>
void
Accumulate::one_cycle (int frames)
{
	d_sample * s = ports[0];
	d_sample glis = pow (2, *ports[1] * frametime);
	d_sample decay = *ports[2];
	d_sample * d = ports[3];

	if (decay == 0) decay = .00001;
	decay = exp (log (decay) * frametime);

	while (frames)
	{
		int n = min (frames, DECFAC - in.fill);

		for (int i = 0; i < n; ++i)
		{
			in.buffer[i + in.fill] = s[i];
			F (d, i, out.buffer[in.fill + i], adding_gain);
		}

		in.fill += n;
		s += n;
		d += n;
		
		if (in.fill == DECFAC)
		{
			in.pvoc.generate_frame (in.buffer, frame, DECFAC, PVOC_AMP_FREQ);
			pv_specaccu (true, true, decay, glis, NBINS, framestore, frame, nyquist);
			out.pvoc.process_frame (frame, out.buffer, PVOC_AMP_FREQ);
			in.fill = 0;
		}

		frames -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Accumulate::port_info [] =
{
	{
		"in",
		INPUT | AUDIO,
		{0}
	}, {
		"glissando",
		INPUT | CONTROL,
		{BOUNDED | DEFAULT_MID, -1, 1}
	}, {
		"decay",
		INPUT | CONTROL,
		{BOUNDED | DEFAULT_LOW, 0, 1}
	}, {
		"out",
		OUTPUT | AUDIO,
		{0}
	}
};

template <>
Descriptor<Accumulate>::Descriptor()
{
	UniqueID = 1793;
	Label = "Accumulate";
	Properties = HARD_RT;

	Name = "spectral accumulator";
	Maker = "Richard Dobson, Trevor Wishart, Tim Goetze <tim@quitte.de>";
	Copyright = "GPL, 1981-2004";

	/* fill port info and vtable */
	autogen();
}

