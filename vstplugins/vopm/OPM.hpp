//opm.hpp

#ifndef __OPM__
#define __OPM__

#include "op.h"
#include "lfo.h"

#define	CMNDBUFSIZE	65535
#define	FlagBufSize	7

#define	OPMLPF_COL	56

class Opm {

	//	short	PcmBuf[PCMBUFSIZE][2];
	//short	(*PcmBuf)[2];
public:
	unsigned int PcmBufSize;
	volatile unsigned int PcmBufPtr;
	int	TotalVolume;	// 
private:



	unsigned int	TimerID;
	char *Author;

	Op	op[8][4];	
	int	EnvCounter1;	
	int	EnvCounter2;
	int	pan[2][N_CH];
	int	CPan[N_CH];
	int	CVol[N_CH];
	int CVel[N_CH];
	Lfo	lfo[N_CH];
	int	SLOTTBL[8*4];
	int	CuCh;			

	unsigned char	CmndBuf[CMNDBUFSIZE+1][2];
	unsigned int	CmndBufPoint[CMNDBUFSIZE+1];


	volatile int	NumCmnd;
	int	CmndReadIdx,CmndWriteIdx;
	int CmndRate;

	inline void SetConnection(int ch, int alg);
	volatile int	OpOut[8];
	int	OpOutDummy;

	double inpopmbuf_dummy;
	short InpOpmBuf0[OPMLPF_COL*2],InpOpmBuf1[OPMLPF_COL*2];
	int InpOpm_idx;
	int OpmLPFidx; short *OpmLPFp;
	double inpadpcmbuf_dummy;

	int	OutOpm[2];
	int InpInpOpm[2],InpOpm[2];
	int	InpInpOpm_prev[2],InpOpm_prev[2];
	int	InpInpOpm_prev2[2],InpOpm_prev2[2];
	int OpmHpfInp[2], OpmHpfInp_prev[2], OpmHpfOut[2];
	int OutInpAdpcm[2],OutInpAdpcm_prev[2],OutInpAdpcm_prev2[2],
		OutOutAdpcm[2],OutOutAdpcm_prev[2],OutOutAdpcm_prev2[2];	

	volatile unsigned char	PpiReg;


	unsigned char	OpmRegNo;		
	unsigned char	OpmRegNo_backup;	

	int Dousa_mode;		

	void (*CallBackFunc)(int sampleframes);


public:
	Opm(void);
	~Opm();

	//inline
	void pcmset22(int,float**,bool);
	inline void MakeTable();
	inline int Start(int samprate, int opmflag, int adpcmflag,
					int betw, int pcmbuf, int late, double rev);
	//inline
	int StartPcm(int samprate, int opmflag, int adpcmflag, int pcmbuf);
	//inline
	int SetSamprate(int samprate);
	inline int SetOpmClock(int clock);
	//inline int WaveAndTimerStart();
	inline int SetOpmWait(int wait);
	inline void CulcCmndRate();
	inline void Reset();
	inline void ResetSamprate();
	inline void Free();

	inline unsigned char OpmPeek();
	//inline void OpmReg(unsigned char no);
	//inline
	void OpmPoke(unsigned char,unsigned char,unsigned int);
	inline void ExecuteCmnd(unsigned int);
	void ForceNoteOff(void);


	//inline
	int SetTotalVolume(int v);

	inline void PushRegs();
	inline void PopRegs();

};

#endif

