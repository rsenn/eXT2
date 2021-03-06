/*
  interface.cc

	LADSPA descriptor factory, host interface.

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

#include <sys/time.h>

#include "basics.h"

#include "Cabinet.h"
#include "Chorus.h"
#include "Phaser.h"
#include "Sin.h"
#include "Lorenz.h"
#include "Roessler.h"
#include "Reverb.h"
#include "Compress.h"
#include "Click.h"
#include "Eq.h"
#include "Clip.h"
#include "White.h"
#include "SweepVF.h"
#include "VCO.h"
#include "Amp.h"
#include "HRTF.h"
#include "Pan.h"
#include "Scape.h"

#include "Descriptor.h"

#define N 33
static DescriptorStub * descriptors [N];

static inline void
seed()
{
	static struct timeval tv;
  gettimeofday (&tv, 0);

	srandom (tv.tv_sec ^ tv.tv_usec);
}

extern "C" {

void caps_init()
{
	DescriptorStub ** d = descriptors;

	*d++ = new Descriptor<Eq>();
	*d++ = new Descriptor<Compress>();
	*d++ = new Descriptor<Pan>();

	*d++ = new Descriptor<PreampIII>();
	*d++ = new Descriptor<PreampIV>();
	*d++ = new Descriptor<AmpIII>();
	*d++ = new Descriptor<AmpIV>();
	*d++ = new Descriptor<AmpV>();
	*d++ = new Descriptor<CabinetI>();
	*d++ = new Descriptor<CabinetII>();
	*d++ = new Descriptor<Clip>();

	*d++ = new Descriptor<ChorusI>();
	*d++ = new Descriptor<StereoChorusI>();
	*d++ = new Descriptor<ChorusII>();
	*d++ = new Descriptor<StereoChorusII>();
	*d++ = new Descriptor<PhaserI>();
	*d++ = new Descriptor<PhaserII>();
	*d++ = new Descriptor<SweepVFI>();
	*d++ = new Descriptor<SweepVFII>();
	*d++ = new Descriptor<Scape>();

	*d++ = new Descriptor<VCOs>();
	*d++ = new Descriptor<VCOd>();
	*d++ = new Descriptor<Chief>();
	*d++ = new Descriptor<Sin>();
	*d++ = new Descriptor<White>();
	*d++ = new Descriptor<Lorenz>();
	*d++ = new Descriptor<Roessler>();

	*d++ = new Descriptor<JVRev>();
	*d++ = new Descriptor<Plate>();
	*d++ = new Descriptor<Plate2x2>();

	*d++ = new Descriptor<Click>();
	*d++ = new Descriptor<Dirac>();
	*d++ = new Descriptor<HRF>();

	seed();
}

void caps_fini()
{
	for (ulong i = 0; i < N; ++i)
		delete descriptors[i];
}

/* /////////////////////////////////////////////////////////////////////// */

const LADSPA_Descriptor *
ladspa_descriptor (unsigned long i)
{
	if (i < N)
		return descriptors[i];

	return 0;
}

}; /* extern "C" */
