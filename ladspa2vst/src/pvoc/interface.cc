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

#include "Plugins.h"
#include "Descriptor.h"

static DescriptorStub * descriptors [3];

#define N (sizeof (descriptors) / sizeof (*descriptors))

extern "C" {

void pvoc_init()
{
	DescriptorStub ** d = descriptors;

	*d++ = new Descriptor<Exaggerate>();
	*d++ = new Descriptor<Transpose>();
	*d++ = new Descriptor<Accumulate>();
}

void pvoc_fini()
{
	for (ulong i = 0; i < N; ++i)
		delete descriptors[i];
}

/* /////////////////////////////////////////////////////////////////////// */

const LADSPA_Descriptor *
ladspa_descriptor (unsigned long i)
{
	if (i >= N)
		return 0;

	return descriptors[i];
}

}; /* extern "C" */
