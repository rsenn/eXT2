/*
    Copyright (C) 2003 Fons Adriaensen
    
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


#include <stdio.h>
#include <math.h>
#include "g2reverb.h"


extern float exp2ap (float x);


void Ladspa_G2reverb::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_G2reverb::active (bool act)
{
    if (! act) _grev->reset ();
}


void Ladspa_G2reverb::runproc (unsigned long len, bool add)
{
    _grev->set_roomsize (_port [4][0]);   
    _grev->set_revbtime (_port [5][0]);   
    _grev->set_ipbandw (0.1 + 0.9 * _port [6][0]);   
    _grev->set_damping (0.9 * _port [7][0]);   
    _grev->set_dryslev (exp2ap (0.1661 * _port [8][0]));   
    _grev->set_refllev (exp2ap (0.1661 * _port [9][0]));   
    _grev->set_taillev (exp2ap (0.1661 * _port [10][0]));   
    _grev->process (len, _port [0], _port[1], _port [2], _port [3]);
}



