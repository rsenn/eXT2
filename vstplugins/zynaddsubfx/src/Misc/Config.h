/*
  ZynAddSubFX - a software synthesizer
 
  Config.h - Configuration file functions
  Copyright (C) 2003-2005 Nasca Octavian Paul
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

#ifndef CONFIG_H
#define CONFIG_H
#include "../globals.h"
#define MAX_STRING_SIZE 4000
#define MAX_BANK_ROOT_DIRS 100

class Config{
    public:
	Config();
	~Config();
	struct {
	    char *LinuxOSSWaveOutDev,*LinuxOSSSeqInDev;
	    int SampleRate,SoundBufferSize,OscilSize,SwapStereo;
	    int WindowsWaveOutId,WindowsMidiInId;
	    int BankUIAutoClose;
	    int DumpNotesToFile,DumpAppend;
	    int GzipCompression;
	    int Interpolation;
	    char *DumpFile;
	    char *bankRootDirList[MAX_BANK_ROOT_DIRS],*currentBankDir;
	    char *presetsDirList[MAX_BANK_ROOT_DIRS];
	    int CheckPADsynth;
	    int UserInterfaceMode;
	    int VirKeybLayout;
	} cfg;
	int winwavemax,winmidimax;//number of wave/midi devices on Windows
	int maxstringsize;
	
	struct winmidionedevice{
	    char *name;
	};
	winmidionedevice *winmididevices;

	void clearbankrootdirlist();
	void clearpresetsdirlist();
	void init();
	void save();
	
    private:
	void readConfig(char *filename);
	void saveConfig(char *filename);
	void getConfigFileName(char *name,int namesize);
};
#endif

