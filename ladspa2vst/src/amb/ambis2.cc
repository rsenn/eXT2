/*
    Copyright (C) 2004 Fons Adriaensen
    
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


#include <string.h>
#include <math.h>
#include "ambis2.h"


#define DEG2RAD 0.0174533f


void Ladspa_Monopan1::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Monopan1::active (bool act)
{
    if (act) calcpar ();
}


void Ladspa_Monopan1::calcpar (void)
{
    float e, a, ce; 

    e = *(_port [CTL_ELEV]) * DEG2RAD;
    _zz = sin (e);
    ce = cos (e);
    a = *(_port [CTL_AZIM]) * DEG2RAD;
    _xx = ce * cos (-a);
    _yy = ce * sin (-a);
}


void Ladspa_Monopan1::runproc (unsigned long len, bool add)
{
    float t, xx, yy, zz, dxx, dyy, dzz; 
    float *in, *out_w, *out_x, *out_y, *out_z;

    xx = _xx;
    yy = _yy;
    zz = _zz;
    calcpar ();
    dxx = (_xx - xx) / len;
    dyy = (_yy - yy) / len;
    dzz = (_zz - zz) / len;

    in = _port [INP];
    out_w = _port [OUT_W];
    out_x = _port [OUT_X];
    out_y = _port [OUT_Y];
    out_z = _port [OUT_Z];

    while (len--)
    {
	xx += dxx;
	yy += dyy;
	zz += dzz;
        t = *in++;        
        *out_w++ = 0.7071f * t;
        *out_x++ = xx * t;
        *out_y++ = yy * t;
        *out_z++ = zz * t;
    }
}



void Ladspa_Stereopan1::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Stereopan1::active (bool act)
{
    if (act) calcpar ();
}


void Ladspa_Stereopan1::calcpar (void)
{
    float e, a, ce; 

    e = _port [CTL_ELEV][0] * DEG2RAD;
    _zz = sin (e);
    ce = cos (e);
    a = (_port [CTL_AZIM][0] - 0.5 * _port [CTL_WIDTH][0]) * DEG2RAD;
    _xl = ce * cos (-a);
    _yl = ce * sin (-a);
    a = (_port [CTL_AZIM][0] + 0.5 * _port [CTL_WIDTH][0]) * DEG2RAD;
    _xr = ce * cos (-a);
    _yr = ce * sin (-a);
}


void Ladspa_Stereopan1::runproc (unsigned long len, bool add)
{
    float  xl, xr, yl, yr, zz, dxl, dxr, dyl, dyr, dzz, u, v; 
    float *in_l, *in_r, *out_w, *out_x, *out_y, *out_z;

    xl = _xl;
    xr = _xr;
    yl = _yl;
    yr = _yr;
    zz = _zz;
    calcpar ();
    dxl = (_xl - xl) / len;
    dxr = (_xr - xr) / len;
    dyl = (_yl - yl) / len;
    dyr = (_yr - yr) / len;
    dzz = (_zz - zz) / len;

    in_l = _port [INP_L];
    in_r = _port [INP_R];
    out_w = _port [OUT_W];
    out_x = _port [OUT_X];
    out_y = _port [OUT_Y];
    out_z = _port [OUT_Z];

    while (len--)
    {
	xl += dxl;
	xr += dxr;
	yl += dyl;
	yr += dyr;
	zz += dzz;
        u = *in_l++;        
        v = *in_r++;        
        *out_w++ = 0.7071f * (u + v);
        *out_z++ = zz * (u + v);
        *out_x++ = xl * u + xr * v;
        *out_y++ = yl * u + yr * v;
    }
}



void Ladspa_Rotator1::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Rotator1::active (bool act)
{
    if (act) calcpar ();
}


void Ladspa_Rotator1::calcpar (void)
{
    float a; 

    a = _port [CTL_AZIM][0] * DEG2RAD;
    _c = cos (a);
    _s = sin (a);
}


void Ladspa_Rotator1::runproc (unsigned long len, bool add)
{

    float c, s, dc, ds, x, y;
    float *in_x, *in_y, *out_x, *out_y;

    memcpy (_port [OUT_W], _port [INP_W], len * sizeof (float));
    memcpy (_port [OUT_Z], _port [INP_Z], len * sizeof (float));

    c = _c;
    s = _s;
    calcpar ();
    dc = (_c - c) / len;
    ds = (_s - s) / len;

    in_x  = _port [INP_X];
    in_y  = _port [INP_Y];
    out_x = _port [OUT_X];
    out_y = _port [OUT_Y];

    while (len--)
    {
	c += dc;
	s += ds;
        x = *in_x++;
        y = *in_y++; 
        *out_x++ = c * x + s * y;
        *out_y++ = c * y - s * x;
    }
}

