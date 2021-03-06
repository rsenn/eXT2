/*
	Clip.cc
	
	Copyright 2003-5 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	simple oversampled hard clipper

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

#include "Clip.h"
#include "Descriptor.h"

void
Clip::init (double _fs)
{
	fs = _fs;
	gain = 1;

	threshold[0] = -.9;
	threshold[1] = +.9;

	/* going a bit lower than nominal with fc */
	double f = .5 * M_PI / OVERSAMPLE;
	
	/* construct the upsampler filter kernel */
	DSP::sinc (f, up.c, FIR_SIZE);
	DSP::kaiser<DSP::apply_window> (up.c, FIR_SIZE, 6.4);

	/* copy upsampler filter kernel for downsampler, make sum */
	double s = 0;
	for (int i = 0; i < up.n; ++i)
		down.c[i] = up.c[i],
		s += up.c[i];
	
	/* scale downsampler kernel for unity gain */
	s = 1 / s;
	for (int i = 0; i < down.n; ++i)
		down.c[i] *= s;

	/* scale upsampler kernel for unity gain */
	s *= OVERSAMPLE;
	for (int i = 0; i < up.n; ++i)
		up.c[i] *= s;
}

inline d_sample
Clip::clip (d_sample a)
{
	if (a < threshold[0])
		return threshold[0];
	if (a > threshold[1])
		return threshold[1];
	return a;
}

template <sample_func_t F>
void
Clip::one_cycle (int frames)
{
	d_sample * s = ports[0];

	double gf;
	if (*ports[1] == gain_db)
		gf = 1;
	else
	{
		gain_db = *ports[1];
		d_sample g = DSP::db2lin (gain_db);
		gf = pow (g / gain, 1 / (double) frames);
	}

	d_sample * d = ports[2];
	*ports[3] = OVERSAMPLE;

	for (int i = 0; i < frames; ++i)
	{
		register d_sample a = gain * s[i];

		a = down.process (clip (up.upsample (a)));

		for (int o = 1; o < OVERSAMPLE; ++o)
			down.store (clip (up.pad (o)));

		F (d, i, a, adding_gain);

		gain *= gf;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Clip::port_info [] = 
{
	{
		"in",
		INPUT | AUDIO,
		{BOUNDED, -1, 1}
	}, {
		"gain (dB)",
		INPUT | CONTROL,
		{BOUNDED | DEFAULT_1, -72, 72}
	}, {
		"out",
		OUTPUT | AUDIO,
		{0}
	}, {
		"latency",
		OUTPUT | CONTROL,
		{0}
	}
};

template <> void
Descriptor<Clip>::setup()
{
	UniqueID = 1771;
	Label = "Clip";
	Properties = HARD_RT;

	Name = "CAPS: Clip - Hard clipper, 8x oversampled";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "GPL, 2003-5";

	/* fill port info and vtable */
	autogen();
}

