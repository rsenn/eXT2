/*
	Plugins.h
	
	Copyright 2004 Tim Goetze <tim@quitte.de>
	Copyright 2001-2 Richard Dobson, Trevor Wishart

	http://quitte.de/dsp/

	phase vocoder plugins designed by Richard Dobson and Trevor Wishart,
	LADSPA adaptation by Tim Goetze

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

#ifndef _PLUGINS_H_
#define _PLUGINS_H_

#include "pvoc.h"

class PvocPlugin
{
	public:
		double fs;
		
		enum {
			DECFAC = 160,
			FFTLEN = 1024,
			NBINS = (FFTLEN + 2) / 2
		};

		int nbins;

		d_sample frame [FFTLEN + 2];
		
		struct {
			d_sample buffer [DECFAC];
			int fill; /* buffer fill, unused in out case (equal to in.fill) */
			phasevocoder pvoc;
		} in, out;

		void init (double _fs);
		void activate();
};

class Exaggerate
: public PvocPlugin
{
	public:
		template <sample_func_t F>
		void one_cycle (int frames);

	public:
		static PortInfo port_info [];
		d_sample * ports [3];

		d_sample adding_gain;

		void init (double _fs);

		void activate();

		void run (int n)
			{
				one_cycle<store_func> (n);
			}
		
		void run_adding (int n)
			{
				one_cycle<adding_func> (n);
			}
};

class Transpose
: public PvocPlugin
{
	public:
		float amp [NBINS], freq [NBINS];

		template <sample_func_t F>
		void one_cycle (int frames);

	public:
		static PortInfo port_info [];
		d_sample * ports [3];

		d_sample adding_gain;

		void init (double _fs);

		void activate();

		void run (int n)
			{
				one_cycle<store_func> (n);
			}
		
		void run_adding (int n)
			{
				one_cycle<adding_func> (n);
			}
};

class Accumulate
: public PvocPlugin
{
	public:
		float framestore [FFTLEN + 2];
		float nyquist, arate, frametime;

		template <sample_func_t F>
		void one_cycle (int frames);

	public:
		static PortInfo port_info [];
		d_sample * ports [4];

		d_sample adding_gain;

		void init (double _fs);

		void activate();

		void run (int n)
			{
				one_cycle<store_func> (n);
			}
		
		void run_adding (int n)
			{
				one_cycle<adding_func> (n);
			}
};

#endif /* _PLUGINS_H_ */
