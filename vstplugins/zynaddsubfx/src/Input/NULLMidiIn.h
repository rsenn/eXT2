/*
  ZynAddSubFX - a software synthesizer
 
  NULLMidiIn.h - a dummy Midi port
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef NULL_MIDI_IN_H
#define NULL_MIDI_IN_H

#include "MidiIn.h"


class NULLMidiIn:public MidiIn{
    public:
	NULLMidiIn();
	~NULLMidiIn();
	void getmidicmd(MidiCmdType &cmdtype,unsigned char &cmdchan,unsigned char *cmdparams);

    private:
};


#endif

