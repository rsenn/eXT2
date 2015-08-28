
#ifndef __OPMDRV__
#define __OPMDRV__

#include <list>
#include "aeffectx.h"
using namespace std;

#define CHMAX 8
#define VOMAX 128

#define ERR_NOERR 0x0000
#define ERR_NOFILE 0x10000000
#define ERR_SYNTAX 0x20000000
#define ERR_LESPRM 0x30000000

#define CHSLOT8 15 //Midi 15Ch assigned OPM8ch

class OPDATA{
	public:
	 //EG-Level
		unsigned int TL;
		unsigned int D1L;
	 //EG-ADR
		unsigned int AR;
		unsigned int D1R;
		unsigned int D2R;
		unsigned int RR;
	 //DeTune
		unsigned int KS;
		unsigned int DT1;
		unsigned int MUL;
		unsigned int DT2;
		unsigned int F_AME;

		OPDATA(void);
};

class CHDATA{
	public:
		unsigned int F_PAN;//L:0x80Msk R:0x40Msk
		unsigned int CON;
		unsigned int FL;
		unsigned int AMS;
		unsigned int PMS;
		unsigned int F_NE;
		unsigned int OpMsk;
		class OPDATA Op[4];
		char Name[16];
		unsigned int SPD;
		unsigned int PMD;
		unsigned int AMD;
		unsigned int WF;
		unsigned int NFQ;
		CHDATA(void);
// ~CHDATA(void);

};


struct TBLMIDICH{
	int VoNum;				
	int Bend;				
	int Vol;				
	int Pan;				
	int BendRang;			
	int PMD;
	int AMD;
	int SPD;
	int OpTL[4];			
	unsigned int RPN;
	unsigned int NRPN;
	bool CuRPN;
	int BfNote;				
	int PortaSpeed;			
	bool PortaOnOff;
	int LFOdelay;
};





class OPMDRV{
// friend class VOPM;
private:
	unsigned int SampleTime;	
	char CuProgName[64];		
	int CuProgNum;				
	int NoisePrm;				
	int MstVol;				
	bool PolyMono;				
	class Opm *pOPM;


	list<int> PlayCh,WaitCh;
	
	struct {
		int VoNum;				
		int MidiCh;			
		int Note;				
		int Vel;				
		int PortaBend;			
		int PortaSpeed;
		int LFOdelayCnt;
	}TblCh[CHMAX];

	struct TBLMIDICH TblMidiCh[16];	

	int AtachCh(void);			
	void DelCh(int);			
	int ChkEnCh(void);			
	int ChkSonCh(int,int);		
	int NonCh(int,int,int);	
	int NoffCh(int,int);		

	void SendVo(int);			
	void SendKc(int);			
	void SendTl(int);			
	void SendPan(int,int);		
	void SendLfo(int);


public:
	OPMDRV(class Opm *);		

	class CHDATA VoTbl[VOMAX];	
															
	 	MidiProgramName MidiProg[16];
	void setDelta(unsigned int);				
	void NoteOn(int Ch,int Note,int Vol);	
	void NoteOff(int Ch,int Note);				
	void NoteAllOff(int Ch);				
	void ForceAllOff(int Ch);						
	void ForceAllOff();								
	void BendCng(int Ch,int Bend);					
	void VolCng(int Ch,int Vol);			
	void VoCng(int Ch,int VoNum);				
	void MsVolCng(int MVol);					
	void PanCng(int Ch,int Pan);			
	int Load(char *);						
	int Save(char *);							
	int getProgNum(void);						
	void setProgNum(long );					
	void setProgName(char* );				
	char* getProgName(void);					
	char* getProgName(int);							
	unsigned char getPrm(int);							
	void setPrm(int,unsigned char);					
	void setRPNH(int,int);
	void setNRPNH(int,int);
	void setRPNL(int,int);
	void setNRPNL(int,int);
	void setData(int,int);
	void setPoly(bool);
	void AutoSeq(int sampleframes);
	void setPortaSpeed(int MidiCh,int speed);
	void setPortaOnOff(int MidiCh,bool onoff);
	void setPortaCtr(int MidiCh,int Note);
	void setLFOdelay(int MidiCh,int data);

	~OPMDRV(void);									
	long getMidiProgram(long channel,MidiProgramName *curProg);
	void ChDelay(int );


	void setCcTL_L(int MidiCh,int data,int OpNum);
	void setCcTL_H(int MidiCh,int data,int OpNum);

	void setCcLFQ_L(int MidiCh,int data);
	void setCcLFQ_H(int MidiCh,int data);

	void setCcPMD_L(int MidiCh,int data);
	void setCcPMD_H(int MidiCh,int data);

	void setCcAMD_L(int MidiCh,int data);
	void setCcAMD_H(int MidiCh,int data);

private:
	int CalcLim8(int VoData,int DifData);
	int CalcLim7(int VoData,int DifData);
	int CalcLimTL(int VoData,int DifData);

	void setAMD(int ch,int Amd);
	void setPMD(int ch,int Pmd);
	void setSPD(int ch,int Spd);
	void setTL(int ch,int Tl,int OpNum);
};

#endif
