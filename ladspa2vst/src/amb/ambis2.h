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


#ifndef __AMBIS2_H
#define __AMBIS2_H


#include "ladspaplugin.h"


class Ladspa_Monopan1 : public LadspaPlugin
{
public:

    enum { INP, OUT_W, OUT_X, OUT_Y, OUT_Z, CTL_ELEV, CTL_AZIM, NPORT  };

    Ladspa_Monopan1 (unsigned long fsam) : LadspaPlugin (fsam) {}
    virtual void setport (unsigned long port, LADSPA_Data *data);  
    virtual void active  (bool act);  
    virtual void runproc (unsigned long len, bool add);
    virtual ~Ladspa_Monopan1 (void) {}  

private:

    void calcpar (void);

    float     *_port [NPORT];
    float      _xx, _yy, _zz;
};



class Ladspa_Stereopan1 : public LadspaPlugin
{
public:

    enum { INP_L, INP_R, OUT_W, OUT_X, OUT_Y, OUT_Z, CTL_ELEV, CTL_WIDTH, CTL_AZIM, NPORT  };

    Ladspa_Stereopan1 (unsigned long fsam) : LadspaPlugin (fsam) {}
    virtual void setport (unsigned long port, LADSPA_Data *data);  
    virtual void active  (bool act);  
    virtual void runproc (unsigned long len, bool add);
    virtual ~Ladspa_Stereopan1 (void) {}  

private:

    void calcpar (void);

    float     *_port [NPORT];
    float      _xl, _xr, _yl, _yr, _zz;
};



class Ladspa_Rotator1 : public LadspaPlugin
{
public:

    enum { INP_W, INP_X, INP_Y, INP_Z, OUT_W, OUT_X, OUT_Y, OUT_Z, CTL_AZIM, NPORT  };

    Ladspa_Rotator1 (unsigned long fsam) : LadspaPlugin (fsam) {}
    virtual void setport (unsigned long port, LADSPA_Data *data);  
    virtual void active  (bool act);  
    virtual void runproc (unsigned long len, bool add);
    virtual ~Ladspa_Rotator1 (void) {}  

private:

    void calcpar (void);

    float     *_port [NPORT];
    float      _c, _s;
};


#endif
