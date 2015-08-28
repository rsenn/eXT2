/*
    Copyright (C) 2004-2006 Fons Adriaensen
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

//-----------------------------------------------------------------------------------
// Common definitions
//-----------------------------------------------------------------------------------


#include "ambis2.h"

#define NMODS 3
#define VERSION "0.1.0"


static const char* maker = "Fons Adriaensen <fons.adriaensen@skynet.be>";
static const char* copyr = "GPL";


static void pconnect (LADSPA_Handle H, unsigned long port, LADSPA_Data *data)
{
    ((LadspaPlugin *)H)->setport (port, data);
}

static void activate (LADSPA_Handle H)
{
    ((LadspaPlugin *)H)->active (true);
}

static void runplugin (LADSPA_Handle H, unsigned long k)
{
    ((LadspaPlugin *)H)->runproc (k, false);
}
/*
static void runadding (LADSPA_Handle H, unsigned long k)
{
    ((LadspaPlugin *)H)->runproc (k, true);
}

static void setadding (LADSPA_Handle H, LADSPA_Data gain)
{
    ((LadspaPlugin *)H)->setgain (gain);
}
*/
static void deactivate (LADSPA_Handle H)
{
    ((LadspaPlugin *)H)->active (false);
}

static void cleanup (LADSPA_Handle H)
{
    delete (LadspaPlugin *) H;
}


//-----------------------------------------------------------------------------------
// Plugin definitions
//-----------------------------------------------------------------------------------


static const char* label0 = "Ambisonics-mono-panner";
static const char* name0  = "AMB Mono to B1-format panner";


static LADSPA_Handle instant0 (const struct _LADSPA_Descriptor *desc, unsigned long rate)
{
    return new Ladspa_Monopan1 (rate);
}


static const LADSPA_PortDescriptor pdesc0 [Ladspa_Monopan1::NPORT] = 
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL
};

static const char * const pname0 [Ladspa_Monopan1::NPORT] = 
{
  "In",
  "Out-W",
  "Out-X",
  "Out-Y",
  "Out-Z",
  "Elevation",
  "Azimuth"
};

static const LADSPA_PortRangeHint phint0 [Ladspa_Monopan1::NPORT] = 
{
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0, -90, 90 },
  { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0, -180, 180 }
};


//-----------------------------------------------------------------------------------


static const char* label1 = "Ambisonics-stero-panner";
static const char* name1  = "AMB Stereo to B1-format panner";


static LADSPA_Handle instant1 (const struct _LADSPA_Descriptor *desc, unsigned long rate)
{
    return new Ladspa_Stereopan1 (rate);
}


static const LADSPA_PortDescriptor pdesc1 [Ladspa_Stereopan1::NPORT] = 
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL
};

static const char * const pname1 [Ladspa_Stereopan1::NPORT] = 
{
  "In-L",
  "In-R",
  "Out-W",
  "Out-X",
  "Out-Y",
  "Out-Z",
  "Elevation",
  "Width",
  "Azimuth"
};

static const LADSPA_PortRangeHint phint1 [Ladspa_Stereopan1::NPORT] = 
{
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0,        -90,  90 },
  { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MAXIMUM,  -90,  90 },
  { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0,       -180, 180 }
};


//-----------------------------------------------------------------------------------


static const char* label2 = "Ambisonics-rotator";
static const char* name2  = "AMB B1-format horizontal rotator";


static LADSPA_Handle instant2 (const struct _LADSPA_Descriptor *desc, unsigned long rate)
{
    return new Ladspa_Rotator1 (rate);
}


static const LADSPA_PortDescriptor pdesc2 [Ladspa_Rotator1::NPORT] = 
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL
};

static const char * const pname2 [Ladspa_Rotator1::NPORT] = 
{
  "In-W",
  "In-X",
  "In-Y",
  "In-Z",
  "Out-W",
  "Out-X",
  "Out-Y",
  "Out-Z",
  "Angle"
};

static const LADSPA_PortRangeHint phint2 [Ladspa_Rotator1::NPORT] = 
{
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0, -180, 180 }
};


//-----------------------------------------------------------------------------------


static const LADSPA_Descriptor moddescr [NMODS] =
{
  {
    1973,
    label0,
    LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE,
    name0,
    maker,
    copyr,
    Ladspa_Monopan1::NPORT,
    pdesc0,
    pname0,
    phint0,
    0,
    instant0,
    pconnect,
    activate,
    runplugin,
    0,
    0,
    deactivate,
    cleanup
  },  
  {
    1974,
    label1,
    LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE,
    name1,
    maker,
    copyr,
    Ladspa_Stereopan1::NPORT,
    pdesc1,
    pname1,
    phint1,
    0,
    instant1,
    pconnect,
    activate,
    runplugin,
    0,
    0,
    deactivate,
    cleanup
  },  
  {
    1975,
    label2,
    LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE,
    name2,
    maker,
    copyr,
    Ladspa_Rotator1::NPORT,
    pdesc2,
    pname2,
    phint2,
    0,
    instant2,
    pconnect,
    activate,
    runplugin,
    0,
    0,
    deactivate,
    cleanup
  }
};


extern "C" const LADSPA_Descriptor *ladspa_descriptor (unsigned long i)
{
  if (i >= NMODS) return 0;
  return moddescr + i;
}


//-----------------------------------------------------------------------------------
