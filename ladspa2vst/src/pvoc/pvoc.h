<<<<<<< HEAD
/*
  pvoc.h

	Copyright (c) 2004 Tim Goetze <tim@quitte.de>
	Copyright (c) 2001-2 Richard Dobson
	Copyright (c) 1981-2004 Regents of the University of California
	
	http://quitte.de/dsp

	phase vocoder: basically Richard Dobson's C++ adaptation of the CARL
	pvoc implementation, adapted for FFTW3. thanks a lot, Richard, thanks
	a lot, Mark Dolson!

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

/* class def for pvoc wrapper */
#ifndef __PVOC_H_INCLUDED
#define __PVOC_H_INCLUDED

#define FFTW3

#ifndef FFTW3
#include <srfftw.h>
#else
#include <fftw3.h>
#endif

#ifndef min
# define min(x,y)  ((x) < (y) ? (x) : (y))
# define max(x,y)  ((x) > (y)  ? (x) : (y))
#endif

typedef enum pvocmode { PVPP_NOT_SET,PVPP_OFFLINE,PVPP_STREAMING};

/* PVOC_INVALID added, tg */
typedef enum pvoc_frametype 
{ 
	PVOC_AMP_FREQ, PVOC_AMP_PHASE, PVOC_COMPLEX, PVOC_INVALID
};

class phasevocoder
{

private:
	double	rratio;
	float	*input,		/* pointer to start of input buffer */
		*output,		/* pointer to start of output buffer */
		*anal,			/* pointer to start of analysis buffer */
		*syn,			/* pointer to start of synthesis buffer */
		*banal,			/* pointer to anal[1] (for FFT calls) */
		*bsyn,			/* pointer to syn[1]  (for FFT calls) */
		*nextIn,		/* pointer to next empty word in input */
		*nextOut,		/* pointer to next empty word in output */
		*analWindow,	/* pointer to center of analysis window */
		*synWindow,		/* pointer to center of synthesis window */
		*maxAmp,		/* pointer to start of max amp buffer */
		*avgAmp,		/* pointer to start of avg amp buffer */
		*avgFrq,		/* pointer to start of avg frq buffer */
		*env,			/* pointer to start of spectral envelope */
		*i0,			/* pointer to amplitude channels */
		*i1,			/* pointer to frequency channels */
		*oi,			/* pointer to old phase channels */
		*oldInPhase,	/* pointer to start of input phase buffer */
		*oldOutPhase;	/* pointer to start of output phase buffer */

	int	m, n;

	int	N ,			/* number of phase vocoder channels (bands) */
		M,			/* length of analWindow impulse response */
		L,			/* length of synWindow impulse response */
		D,			/* decimation factor (default will be M/8) */
		I,			/* interpolation factor (default will be I=D)*/
		W , 		/* filter overlap factor (determines M, L) */
		/*RWD: want EXACT frequency! */
	//	F,			/* fundamental frequency (determines N) */
	//	F2,			/* F/2 */	   /* RWD NOT USED */
		analWinLen,		/* half-length of analysis window */
		synWinLen;		/* half-length of synthesis window */
	/* RWD see above */
	float Fexact;
	long //	tmprate,	/* temporary variable */
		sampsize,		/* sample size for output file */
		//origrate,		/* sample rate of file analysed */
		outCount,		/* number of samples written to output */
		ibuflen,		/* length of input buffer */
		obuflen,		/* length of output buffer */
		nI,			/* current input (analysis) sample */
		nO,				/* current output (synthesis) sample */
		nMaxOut,		/* last output (synthesis) sample */
		nMin,		/* first input (analysis) sample */
		nMax;	/* last input sample (unless EOF) */
/***************************** 6:2:91  OLD CODE **************
						long	origsize;
*******************************NEW CODE **********************/
	long	origsize;	/* sample type of file analysed */

	char	ch;		/* needed for crack (commandline interpreter)*/
	
	int	ifd, ofd;	/* CDP sound file handles */
	float	beta ,	/* parameter for Kaiser window */
		real,		/* real part of analysis data */
		imag,		/* imaginary part of analysis data */
		mag,		/* magnitude of analysis data */
		phase,		/* phase of analysis data */
		angleDif,	/* angle difference */
		RoverTwoPi,	/* R/D divided by 2*Pi */
		TwoPioverR,	/* 2*Pi divided by R/I */
		sum,		/* scale factor for renormalizing windows */
		ftot,	/* scale factor for calculating statistics */
		rIn,		/* decimated sampling rate */
		rOut,		/* pre-interpolated sampling rate */
		invR,		/* 1. / srate */
		time,		/* nI / srate */
		tvx0,		/* current x value of time-var function */
		tvx1,		/* next x value of time-var function */
		tvdx,		/* tvx1 - tvx0 */
		tvy0,		/* current y value of time-var function */
		tvy1,		/* next y value of time-var function */
		tvdy,		/* tvy1 - tvy0 */
		frac,		/* tvdy / tvdx */
		warp,	/* spectral envelope warp factor */
		R ,		/* input sampling rate */
		P ,		/* pitch scale factor */
		Pinv,		/* 1. / P */
		T;		/* time scale factor ( >1 to expand)*/

	int	i,j,k,		/* index variables */
		Dd,		/* number of new inputs to read (Dd <= D) */
		Ii,		/* number of new outputs to write (Ii <= I) */
		N2,		/* N/2 */
		NO,		/* synthesis NO = N / P */
		NO2,		/* NO/2 */
		IO,		/* synthesis IO = I / P */
		IOi,		/* synthesis IOi = Ii / P */
		//Mlen,
		Mf,		/* flag for even M */
		Lf,		/* flag for even L */
		//Dfac,
		flag,		/* end-of-input flag */
		C,		/* flag for resynthesizing even or odd chans */
		Ci,		/* flag for resynthesizing chans i to j only */
		Cj,		/* flag for resynthesizing chans i to j only */
		CC,		/* flag for selected channel resynthesis */
		X,		/* flag for magnitude output */
		E,		/* flag for spectral envelope output */	
		tvflg,	/* flag for time-varying time-scaling */
		tvnxt,		/* counter for stepping thru time-var func */
		tvlen;		/* length of time-varying function */
	
	float	srate;		/* sample rate from header on stdin */
		
	float	timecheckf;
	long	isr,			/* sampling rate */
	Nchans;			/* no of chans */

	/* my vars */
	int vH;				/* von Hann window */
	pvocmode m_mode;
	int bin_index;		/* to spread norm_phase over many frames */
	float *synWindow_base;
	float *analWindow_base;

#ifdef FFTW3
	fftwf_plan forward_plan, inverse_plan;
#else
	rfftwnd_plan forward_plan, inverse_plan;
#endif
	int in_fftw_size,out_fftw_size;
	float Ninv;

protected:
	double besseli( double x);			 /* from Csound*/
	void hamming(float *win,int winLen,int even);
	void kaiser(float *win,int len,double Beta);     /* from Csound */
	void vonhann(float *win,int winLen,int even);
	

public:
	phasevocoder();
	virtual ~phasevocoder();
	bool init(long srate,long fftlen,long decfac,pvocmode mode);
	long anal_overlap(void) { return D;}
	/*retval gives numsamps written to outbuf, -1 for error */
	long process_frame(float *frame,float *outbuf,pvoc_frametype frametype);	
	long process_frame(float *frame,short *outbuf);
	long generate_frame(float *fbuf,float *outanal,long samps,pvoc_frametype frametype);

	void scale_synwindow (float factor);
};




#endif
=======
/*
  pvoc.h

	Copyright (c) 2004 Tim Goetze <tim@quitte.de>
	Copyright (c) 2001-2 Richard Dobson
	Copyright (c) 1981-2004 Regents of the University of California
	
	http://quitte.de/dsp

	phase vocoder: basically Richard Dobson's C++ adaptation of the CARL
	pvoc implementation, adapted for FFTW3. thanks a lot, Richard, thanks
	a lot, Mark Dolson!

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

/* class def for pvoc wrapper */
#ifndef __PVOC_H_INCLUDED
#define __PVOC_H_INCLUDED

#define FFTW3

#ifndef FFTW3
#include <srfftw.h>
#else
#include <fftw3.h>
#endif

#ifndef min
# define min(x,y)  ((x) < (y) ? (x) : (y))
# define max(x,y)  ((x) > (y)  ? (x) : (y))
#endif

typedef enum pvocmode { PVPP_NOT_SET,PVPP_OFFLINE,PVPP_STREAMING};

/* PVOC_INVALID added, tg */
typedef enum pvoc_frametype 
{ 
	PVOC_AMP_FREQ, PVOC_AMP_PHASE, PVOC_COMPLEX, PVOC_INVALID
};

class phasevocoder
{

private:
	double	rratio;
	float	*input,		/* pointer to start of input buffer */
		*output,		/* pointer to start of output buffer */
		*anal,			/* pointer to start of analysis buffer */
		*syn,			/* pointer to start of synthesis buffer */
		*banal,			/* pointer to anal[1] (for FFT calls) */
		*bsyn,			/* pointer to syn[1]  (for FFT calls) */
		*nextIn,		/* pointer to next empty word in input */
		*nextOut,		/* pointer to next empty word in output */
		*analWindow,	/* pointer to center of analysis window */
		*synWindow,		/* pointer to center of synthesis window */
		*maxAmp,		/* pointer to start of max amp buffer */
		*avgAmp,		/* pointer to start of avg amp buffer */
		*avgFrq,		/* pointer to start of avg frq buffer */
		*env,			/* pointer to start of spectral envelope */
		*i0,			/* pointer to amplitude channels */
		*i1,			/* pointer to frequency channels */
		*oi,			/* pointer to old phase channels */
		*oldInPhase,	/* pointer to start of input phase buffer */
		*oldOutPhase;	/* pointer to start of output phase buffer */

	int	m, n;

	int	N ,			/* number of phase vocoder channels (bands) */
		M,			/* length of analWindow impulse response */
		L,			/* length of synWindow impulse response */
		D,			/* decimation factor (default will be M/8) */
		I,			/* interpolation factor (default will be I=D)*/
		W , 		/* filter overlap factor (determines M, L) */
		/*RWD: want EXACT frequency! */
	//	F,			/* fundamental frequency (determines N) */
	//	F2,			/* F/2 */	   /* RWD NOT USED */
		analWinLen,		/* half-length of analysis window */
		synWinLen;		/* half-length of synthesis window */
	/* RWD see above */
	float Fexact;
	long //	tmprate,	/* temporary variable */
		sampsize,		/* sample size for output file */
		//origrate,		/* sample rate of file analysed */
		outCount,		/* number of samples written to output */
		ibuflen,		/* length of input buffer */
		obuflen,		/* length of output buffer */
		nI,			/* current input (analysis) sample */
		nO,				/* current output (synthesis) sample */
		nMaxOut,		/* last output (synthesis) sample */
		nMin,		/* first input (analysis) sample */
		nMax;	/* last input sample (unless EOF) */
/***************************** 6:2:91  OLD CODE **************
						long	origsize;
*******************************NEW CODE **********************/
	long	origsize;	/* sample type of file analysed */

	char	ch;		/* needed for crack (commandline interpreter)*/
	
	int	ifd, ofd;	/* CDP sound file handles */
	float	beta ,	/* parameter for Kaiser window */
		real,		/* real part of analysis data */
		imag,		/* imaginary part of analysis data */
		mag,		/* magnitude of analysis data */
		phase,		/* phase of analysis data */
		angleDif,	/* angle difference */
		RoverTwoPi,	/* R/D divided by 2*Pi */
		TwoPioverR,	/* 2*Pi divided by R/I */
		sum,		/* scale factor for renormalizing windows */
		ftot,	/* scale factor for calculating statistics */
		rIn,		/* decimated sampling rate */
		rOut,		/* pre-interpolated sampling rate */
		invR,		/* 1. / srate */
		time,		/* nI / srate */
		tvx0,		/* current x value of time-var function */
		tvx1,		/* next x value of time-var function */
		tvdx,		/* tvx1 - tvx0 */
		tvy0,		/* current y value of time-var function */
		tvy1,		/* next y value of time-var function */
		tvdy,		/* tvy1 - tvy0 */
		frac,		/* tvdy / tvdx */
		warp,	/* spectral envelope warp factor */
		R ,		/* input sampling rate */
		P ,		/* pitch scale factor */
		Pinv,		/* 1. / P */
		T;		/* time scale factor ( >1 to expand)*/

	int	i,j,k,		/* index variables */
		Dd,		/* number of new inputs to read (Dd <= D) */
		Ii,		/* number of new outputs to write (Ii <= I) */
		N2,		/* N/2 */
		NO,		/* synthesis NO = N / P */
		NO2,		/* NO/2 */
		IO,		/* synthesis IO = I / P */
		IOi,		/* synthesis IOi = Ii / P */
		//Mlen,
		Mf,		/* flag for even M */
		Lf,		/* flag for even L */
		//Dfac,
		flag,		/* end-of-input flag */
		C,		/* flag for resynthesizing even or odd chans */
		Ci,		/* flag for resynthesizing chans i to j only */
		Cj,		/* flag for resynthesizing chans i to j only */
		CC,		/* flag for selected channel resynthesis */
		X,		/* flag for magnitude output */
		E,		/* flag for spectral envelope output */	
		tvflg,	/* flag for time-varying time-scaling */
		tvnxt,		/* counter for stepping thru time-var func */
		tvlen;		/* length of time-varying function */
	
	float	srate;		/* sample rate from header on stdin */
		
	float	timecheckf;
	long	isr,			/* sampling rate */
	Nchans;			/* no of chans */

	/* my vars */
	int vH;				/* von Hann window */
	pvocmode m_mode;
	int bin_index;		/* to spread norm_phase over many frames */
	float *synWindow_base;
	float *analWindow_base;

#ifdef FFTW3
	fftwf_plan forward_plan, inverse_plan;
#else
	rfftwnd_plan forward_plan, inverse_plan;
#endif
	int in_fftw_size,out_fftw_size;
	float Ninv;

protected:
	double besseli( double x);			 /* from Csound*/
	void hamming(float *win,int winLen,int even);
	void kaiser(float *win,int len,double Beta);     /* from Csound */
	void vonhann(float *win,int winLen,int even);
	

public:
	phasevocoder();
	virtual ~phasevocoder();
	bool init(long srate,long fftlen,long decfac,pvocmode mode);
	long anal_overlap(void) { return D;}
	/*retval gives numsamps written to outbuf, -1 for error */
	long process_frame(float *frame,float *outbuf,pvoc_frametype frametype);	
	long process_frame(float *frame,short *outbuf);
	long generate_frame(float *fbuf,float *outanal,long samps,pvoc_frametype frametype);

	void scale_synwindow (float factor);
};




#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
