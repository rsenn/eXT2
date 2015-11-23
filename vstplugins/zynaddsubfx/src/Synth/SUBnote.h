/*
  ZynAddSubFX - a software synthesizer
 
  SUBnote.h - The subtractive synthesizer
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

#ifndef SUB_NOTE_H
#define SUB_NOTE_H

#include "../globals.h"
#include "../Params/SUBnoteParameters.h"
#include "../Params/Controller.h"
#include "Envelope.h"
#include "../DSP/Filter.h"

class SUBnote{
    public:
	SUBnote(SUBnoteParameters *parameters,Controller *ctl_,REALTYPE freq,REALTYPE velocity,int portamento_,int midinote);
	~SUBnote();
	int noteout(REALTYPE *outl,REALTYPE *outr);//note output,return 0 if the note is finished
	void relasekey();
	int finished();
		
	int ready; //if I can get the sampledata
	
    private:
	
	void computecurrentparameters();	
	void initparameters(REALTYPE freq);	
	void KillNote();
    
	SUBnoteParameters *pars;

	//parameters
        int stereo;
	int numstages;//number of stages of filters
	int numharmonics;//number of harmonics (after the too higher hamonics are removed)
	int start;//how the harmonics start
	REALTYPE basefreq;
	REALTYPE panning;
	Envelope *AmpEnvelope;
	Envelope *FreqEnvelope;
	Envelope *BandWidthEnvelope;

	Filter *GlobalFilterL,*GlobalFilterR;
    
	Envelope *GlobalFilterEnvelope;
	
	//internal values	
	ONOFFTYPE NoteEnabled;
	int firsttick,portamento;
	REALTYPE volume,oldamplitude,newamplitude;

	REALTYPE GlobalFilterCenterPitch;//octaves
	REALTYPE GlobalFilterFreqTracking;

        struct bpfilter{
	    REALTYPE freq,bw,amp; //filter parameters
	    REALTYPE a1,a2,b0,b2;//filter coefs. b1=0
	    REALTYPE xn1,xn2,yn1,yn2;  //filter internal values
	};

	void initfilter(bpfilter &filter,REALTYPE freq,REALTYPE bw,REALTYPE amp,REALTYPE mag);
	void computefiltercoefs(bpfilter &filter,REALTYPE freq,REALTYPE bw,REALTYPE gain);
	void filter(bpfilter &filter,REALTYPE *smps);

	bpfilter *lfilter,*rfilter;
	
	REALTYPE *tmpsmp;
	REALTYPE *tmprnd;//this is filled with random numbers
	
	Controller *ctl;
	int oldpitchwheel,oldbandwidth;
	REALTYPE globalfiltercenterq;
	
};




#endif

