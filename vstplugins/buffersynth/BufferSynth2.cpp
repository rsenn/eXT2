//	BufferSynth2.cpp
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include <stdio.h>       //for the printf commands used for the parameter displays
#include <math.h>

#include "BufferSynth2.h"
#include "EndianSwapFunctions.h"

extern bool oome;

//----------------------------------------------------------------------------
//Plugin Constructor - create programs, set certain variables
//----------------------------------------------------------------------------
BufferSynth2::BufferSynth2(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	int i;
	unsigned long j;

	editor = 0;
	pd = 0;
	NoteMaster = 0;
	EdArraySize = 0;

	//sprintf(patchPath, "");

	InitEndian();

	pd = new BS2PresetData;

	Buffer1 = new float *[kNumPrograms];
	Buffer2 = new float *[kNumPrograms];
	for(i=0;i<kNumPrograms;i++)
	{
		Buffer1[i] = new float[BUFFERSIZE];
		Buffer2[i] = new float[BUFFERSIZE];
		for(j=0;j<BUFFERSIZE;j++)
		{
			Buffer1[i][j] = 0.0f;
			Buffer2[i][j] = 0.0f;
		}
	}

	NoteMaster = new bs2notemaster(samplerate);
	
	if(pd)
	{
		SetPatches();
	}
		
	strcpy(kEffectName, "Buffer Synth 2");
	strcpy(kProduct, "ndc Plugs BufferSynth2");
	strcpy(kVendor, "ndc Plugs");
	
	hasVu(kVU);
    setNumInputs(kNumInputs);
    setNumOutputs(kNumOutputs);
    canMono(kCanMono);
    canProcessReplacing(kCanReplacing);
    isSynth(kIsSynth);
    setUniqueID(kID);

	programsAreChunks(true);

	tempo = 125.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;
	NoteMaster->SetSamplerate(samplerate);

	checkTooSoon = 0;
	checkTempo = 1024;
	checkBSZero = false;

	CurrentParam = 0;

	FiltFilter = new ParameterFilter(samplerate, 0.0022675736961451247165532879818594);	//0.0022675736961451247165532879818594 = 100/44100

	ThreshEnv1 = 0.0f;
	ThreshEnv2 = 0.0f;
	threshMeterCount = 0;
 }

//----------------------------------------------------------------------------
//Plugin Destructor - destroy programs, destroy buffers if you've got them
//----------------------------------------------------------------------------
BufferSynth2::~BufferSynth2()
{
	int i;

	if(FiltFilter)
		delete FiltFilter;

	if(NoteMaster)
		delete NoteMaster;
	if(pd)
		delete pd;
	pd = 0;

	for(i=0;i<kNumPrograms;i++)
	{
		delete [] Buffer1[i];
		delete [] Buffer2[i];
	}
	delete [] Buffer1;
	delete [] Buffer2;
}

//----------------------------------------------------------------------------
//Where the processing takes place.
//Saves having to type the same thing into both process and processreplacing at
//same time.
//----------------------------------------------------------------------------
twofloats BufferSynth2::DoProcess(twofloats a, bool barStart)
{
	twofloats retval;
	float tempfilt;

	if(ffilt_OnOff > 0.5f)
	{
		tempfilt = (float)FiltFilter->GetSample(ffilt_Cutoff);
		NoteMaster->SetParameter(kfilt_Cutoff, tempfilt);
	}

	//--Calculate threshold envelope--------
	if(fb1_Input < (1.0f/3.0f))
	{
		if((fabs(a.left)*fb1_IPGain*2.0f) > ThreshEnv1)
			ThreshEnv1 = ((float)fabs(a.left)*fb2_IPGain*2.0f);
		else/* if((fabs(in1[0]*fb1_IPGain*2.0f) > 0.0f))*/
			ThreshEnv1 -= 0.0001f;
	}
	else if(fb1_Input < (2.0f/3.0f))
	{
		if((fabs(a.right)*fb1_IPGain*2.0f) > ThreshEnv1)
			ThreshEnv1 = ((float)fabs(a.right)*fb2_IPGain*2.0f);
		else/* if((fabs(in2[0]*fb1_IPGain*2.0f) > 0.0f))*/
			ThreshEnv1 -= 0.0001f;
	}
	if(fb2_Input < (1.0f/3.0f))
	{
		if((fabs(a.left)*fb2_IPGain*2.0f) > ThreshEnv2)
			ThreshEnv2 = ((float)fabs(a.left)*fb2_IPGain*2.0f);
		else/* if((fabs(in1[0]*fb2_IPGain*2.0f) > 0.0f))*/
			ThreshEnv2 -= 0.0001f;
	}
	else if(fb2_Input < (2.0f/3.0f))
	{
		if((fabs(a.right)*fb2_IPGain*2.0f) > ThreshEnv2)
			ThreshEnv2 = ((float)fabs(a.right)*fb2_IPGain*2.0f);
		else/* if((fabs(in2[0]*fb2_IPGain*2.0f) > 0.0f))*/
			ThreshEnv2 -= 0.0001f;
	}
	if(ThreshEnv1 < 0.0f)
		ThreshEnv1 = 0.0f;
	if(ThreshEnv2 < 0.0f)
		ThreshEnv2 = 0.0f;

	//--Write the i/p to the buffers--------
	if(fb1_Freeze < 0.5f)
	{
		if(fb1_Input < (1.0f/3.0f))
		{
			a.left *= fb1_IPGain * 2.0f;
			if(ThreshEnv1 >= fb1_RecThreshold)
			{
				NoteMaster->Write2IPBuffer1(a.left);
			}
		}
		else if(fb1_Input < (2.0f/3.0f))
		{
			a.right *= fb1_IPGain * 2.0f;
			if(ThreshEnv2 >= fb1_RecThreshold)
			{
				NoteMaster->Write2IPBuffer1(a.right);
			}
		}
	}

	if(fb2_Freeze < 0.5f)
	{
		if(fb2_Input < (1.0f/3.0f))
		{
			a.left *= fb2_IPGain * 2.0f;
			if(ThreshEnv1 >= fb2_RecThreshold)
				NoteMaster->Write2IPBuffer2(a.left);
		}
		else if(fb2_Input < (2.0f/3.0f))
		{
			a.right *= fb2_IPGain * 2.0f;
			if(ThreshEnv2 >= fb2_RecThreshold)
				NoteMaster->Write2IPBuffer2(a.right);
		}
	}

	//--Get the current samples (left & right)--------
	retval = NoteMaster->GetSample(barStart);

	retval = (retval*fop_Mix)+(a*(1.0f-fop_Mix));
	retval *= fop_Level;

	return retval;
}

//----------------------------------------------------------------------------
//Shouldn't need to change this...
//----------------------------------------------------------------------------
void BufferSynth2::process(float **inputs, float **outputs, long sampleFrames)
{
	unsigned long i;
	double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	unsigned long remainingSamples = 0;
	bool barStart = false;
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats a;

	tempo = (float)((float)tempoAt(0)*0.0001f);
	NoteMaster->SetTempo(tempo);

	VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
	if(TimeInfo)
	{
		bs = TimeInfo->barStartPos;
		ppq = TimeInfo->ppqPos;
		numerator = TimeInfo->timeSigNumerator;
		denominator = TimeInfo->timeSigDenominator;

		if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
			barStart = false;
		else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
			barStart = false;
		else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
			barStart = false;
		else
		{
			currentPPQ = ppq-bs;
			if(currentPPQ > 0.0)
			{
				quartersPerBar = ((1.0/denominator)*4.0)*numerator;
				remainingTime = (60.0/static_cast<double>(tempo))*(quartersPerBar-currentPPQ);
				remainingSamples = static_cast<unsigned long>(remainingTime * samplerate);
				if(remainingSamples < static_cast<unsigned long>(sampleFrames))
					barStart = true;
			}
			else
				barStart = true;
		}
	}

	for(i=0;i<static_cast<unsigned long>(sampleFrames);i++)
	{
		a.left = in1[i];
		a.right = in2[i];

		if(barStart)
		{
			if(remainingSamples == i)
				a = DoProcess(a, true);
			else
				a = DoProcess(a, false);
		}
		else
			a = DoProcess(a, false);
        
		out1[i] += a.left; //sums input & processed data
		out2[i] += a.right;
	}
}

//----------------------------------------------------------------------------
//Shouldn't need to change this...
//----------------------------------------------------------------------------
void BufferSynth2::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	unsigned long i;
	double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	unsigned long remainingSamples = 0;
	bool barStart = false;
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats a;

	tempo = (float)((float)tempoAt(0)*0.0001f);
	NoteMaster->SetTempo(tempo);

	/*if(getBarStart())
		barStart = true;*/

	VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
	if(TimeInfo)
	{
		bs = TimeInfo->barStartPos;
		ppq = TimeInfo->ppqPos;
		numerator = TimeInfo->timeSigNumerator;
		denominator = TimeInfo->timeSigDenominator;

		if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
			barStart = false;
		else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
			barStart = false;
		else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
			barStart = false;
		else
		{
			currentPPQ = ppq-bs;
			if(currentPPQ > 0.0)
			{
				quartersPerBar = ((1.0/denominator)*4.0)*numerator;
				remainingTime = (60.0/static_cast<double>(tempo))*(quartersPerBar-currentPPQ);
				remainingSamples = static_cast<unsigned long>(remainingTime * samplerate);
				if(remainingSamples < static_cast<unsigned long>(sampleFrames))
					barStart = true;
			}
			else
				barStart = true;
		}
	}

	for(i=0;i<static_cast<unsigned long>(sampleFrames);i++)
	{
		a.left = in1[i];
		a.right = in2[i];

		if(barStart)
		{
			if(remainingSamples == i)
				a = DoProcess(a, true);
			else
				a = DoProcess(a, false);
		}
		else
			a = DoProcess(a, false);
        
		out1[i] = a.left;
		out2[i] = a.right;
	}
}

//----------------------------------------------------------------------------
void BufferSynth2::setWDArray(int size)
{
	EdArraySize = size;
}

//----------------------------------------------------------------------------
void BufferSynth2::getWDArray(float *buf, int num)
{
	if(EdArraySize == 0)
		return;

	if(num == 0)
		NoteMaster->GetBuffer1(buf, EdArraySize);
	else
		NoteMaster->GetBuffer2(buf, EdArraySize);
}

//----------------------------------------------------------------------------
void BufferSynth2::Write2Buffer1(char *path)
{
    float *temp = 0;
    
    temp = Loader.LoadWaveFile(path, samplerate, fb1_StretchFile, 44100);
    if(temp)
	{
        NoteMaster->Write2IPBuffer1(temp);
		delete temp;
	}
}

//----------------------------------------------------------------------------
void BufferSynth2::Write2Buffer2(char *path)
{
    float *temp;
    
    temp = Loader.LoadWaveFile(path, samplerate, fb2_StretchFile, 44100);
    if(temp)
	{
        NoteMaster->Write2IPBuffer2(temp);
		delete temp;
	}
}

//----------------------------------------------------------------------------
void BufferSynth2::LoadPatch(char *path)
{
	unsigned long i;
	FILE *fp;
	BS2Preset *temppreset;

#ifndef MAC
	fp = fopen(path, "rb");
#else
        char *tempfilename;
        tempfilename = path+12;
        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == ':')
                tempfilename[i] = '/';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }

        fp = fopen(tempfilename, "rb");

        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == '/')
                tempfilename[i] = ':';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }
#endif

	temppreset = &(pd->presets[curProgram]);
	//temppreset += curProgram;

	fread(temppreset, sizeof(BS2Preset), 1, fp);
	pd->presets[curProgram].EndianSwap();

	if(pd->presets[curProgram].sett_SaveBufferContents.val > 0.5f)
	{
		fread(Buffer1[curProgram], sizeof(float), BUFFERSIZE, fp);
		fread(Buffer2[curProgram], sizeof(float), BUFFERSIZE, fp);
		for(i=0;i<BUFFERSIZE;i++)
		{
			Buffer1[curProgram][i] = LittleFloat(Buffer1[curProgram][i]);
			Buffer2[curProgram][i] = LittleFloat(Buffer2[curProgram][i]);
		}

		NoteMaster->Write2IPBuffer1(Buffer1[curProgram]);
		NoteMaster->Write2IPBuffer2(Buffer2[curProgram]);
	}

	fclose(fp);

	setProgram(curProgram);
	updateDisplay();
}

//----------------------------------------------------------------------------
void BufferSynth2::SavePatch(char *path)
{
	unsigned long i;
	FILE *fp;

	fp = NULL;

	NoteMaster->GetBuffer1(Buffer1[curProgram]);	//i.e. we save the contents of the current buffers before we switch patches
	NoteMaster->GetBuffer2(Buffer2[curProgram]);

#ifndef MAC
	fp = fopen(path, "wb");
#else //hmm... what does this do again?
        char *tempfilename;
        tempfilename = path+12;
        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == ':')
                tempfilename[i] = '/';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }

        fp = fopen(tempfilename, "wb");

        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == '/')
                tempfilename[i] = ':';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }
#endif
	if(!fp)
		return;

	if(fsett_SaveBufferContents > 0.5f)
		strcpy(pd->presets[curProgram].patchPath, path);
	else
		strcpy(pd->presets[curProgram].patchPath, "");

	pd->presets[curProgram].EndianSwap();
	fwrite(&(pd->presets[curProgram]), sizeof(BS2Preset), 1, fp);
	pd->presets[curProgram].EndianSwap();	//to swap it back to what it's meant to be

	if(pd->presets[curProgram].sett_SaveBufferContents.val > 0.5f)
	{
		for(i=0;i<BUFFERSIZE;i++)
		{
			Buffer1[curProgram][i] = LittleFloat(Buffer1[curProgram][i]);
			Buffer2[curProgram][i] = LittleFloat(Buffer2[curProgram][i]);

			/*fwrite(Buffer1[curProgram][i], sizeof(float), 1, fp);
			fwrite(Buffer2[curProgram][i], sizeof(float), 1, fp);

			Buffer1[curProgram][i] = LittleFloat(Buffer1[curProgram][i]);//ditto
			Buffer2[curProgram][i] = LittleFloat(Buffer2[curProgram][i]);*/
		}
		fwrite(Buffer1[curProgram], sizeof(float), BUFFERSIZE, fp);
		fwrite(Buffer2[curProgram], sizeof(float), BUFFERSIZE, fp);
		for(i=0;i<BUFFERSIZE;i++)
		{
			Buffer1[curProgram][i] = LittleFloat(Buffer1[curProgram][i]);//ditto
			Buffer2[curProgram][i] = LittleFloat(Buffer2[curProgram][i]);
		}
	}

	fclose(fp);
}

//----------------------------------------------------------------------------
void BufferSynth2::LoadBank(char *path)
{
	unsigned long i, j;
	FILE *fp;
	BS2Preset *temppreset;

#ifndef MAC
	fp = fopen(path, "rb");
#else
        char *tempfilename;
        tempfilename = path+12;
        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == ':')
                tempfilename[i] = '/';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }

        fp = fopen(tempfilename, "rb");

        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == '/')
                tempfilename[i] = ':';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }
#endif

	//temppreset = &(pd->presets[curProgram]);
	//temppreset += curProgram;

	for(j=0;j<kNumPrograms;j++)
	{
		temppreset = &(pd->presets[j]);

		fread(temppreset, sizeof(BS2Preset), 1, fp);
		pd->presets[j].EndianSwap();

		if(pd->presets[j].sett_SaveBufferContents.val > 0.5f)
		{
			fread(Buffer1[j], sizeof(float), BUFFERSIZE, fp);
			fread(Buffer2[j], sizeof(float), BUFFERSIZE, fp);
			for(i=0;i<BUFFERSIZE;i++)
			{
				Buffer1[j][i] = LittleFloat(Buffer1[j][i]);
				Buffer2[j][i] = LittleFloat(Buffer2[j][i]);
			}

			if(j == (unsigned long)curProgram)
			{
				NoteMaster->Write2IPBuffer1(Buffer1[j]);
				NoteMaster->Write2IPBuffer2(Buffer2[j]);
			}
		}
	}

	fclose(fp);

	setProgram(curProgram);
	updateDisplay();
}

//----------------------------------------------------------------------------
void BufferSynth2::SaveBank(char *path)
{
	unsigned long i, j;
	FILE *fp;

	fp = NULL;

	NoteMaster->GetBuffer1(Buffer1[curProgram], BUFFERSIZE);	//i.e. we save the contents of the current buffers before we switch patches
	NoteMaster->GetBuffer2(Buffer2[curProgram], BUFFERSIZE);

#ifndef MAC
	fp = fopen(path, "wb");
#else
        char *tempfilename;
        tempfilename = path+12;
        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == ':')
                tempfilename[i] = '/';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }

        fp = fopen(tempfilename, "wb");

        i = 0;
        while((tempfilename[i] != 0)||(tempfilename[i] != '\n'))
        {
            if(tempfilename[i] == '/')
                tempfilename[i] = ':';
                
            i++;
            
            if(i > 1024)
            {
                break;
                tempfilename[i] = 0;
            }
        }
#endif

	for(j=0;j<kNumPrograms;j++)
	{
		pd->presets[j].EndianSwap();
		fwrite(&(pd->presets[j]), sizeof(BS2Preset), 1, fp);
		pd->presets[j].EndianSwap();	//to swap it back to what it's meant to be

		if(pd->presets[j].sett_SaveBufferContents.val > 0.5f)
		{
			for(i=0;i<BUFFERSIZE;i++)
			{
				Buffer1[j][i] = LittleFloat(Buffer1[j][i]);
				Buffer2[j][i] = LittleFloat(Buffer2[j][i]);
			}

			fwrite(Buffer1[j], sizeof(float), BUFFERSIZE, fp);
			fwrite(Buffer2[j], sizeof(float), BUFFERSIZE, fp);

			for(i=0;i<BUFFERSIZE;i++)
			{
				Buffer1[j][i] = LittleFloat(Buffer1[j][i]);//ditto
				Buffer2[j][i] = LittleFloat(Buffer2[j][i]);
			}
		}
	}

	fclose(fp);
}

//----------------------------------------------------------------------------
void BufferSynth2::IncPatch()
{
	if((curProgram+1)<kNumPrograms)
		setProgram(curProgram+1);
	else
		setProgram(0);
	updateDisplay();
}

//----------------------------------------------------------------------------
void BufferSynth2::DecPatch()
{
	if((curProgram-1)>-1)
		setProgram(curProgram-1);
	else
		setProgram(kNumPrograms-1);
	updateDisplay();
}

//#if defined(_WINDOWS) && defined(_DEBUG)
#ifdef TOMASZ_DEBUG
//----------------------------------------------------------------------------
void BufferSynth2::Save2Cpp()
{
	unsigned long i;
	char temp[24];

	FILE *fp;

	fp = fopen("D:\\Program Files\\Microsoft Visual Studio\\MyProjects\\VST Plugins\\Buffer Synth 2\\BS2InternalPatch.cpp", "w");

	fprintf(fp, "	setParameter(kb1_Start, %ff);\n", fb1_Start);
	fprintf(fp, "	setParameter(kb1_End, %ff);\n", fb1_End);
	fprintf(fp, "	setParameter(kb1_Size, %ff);\n", fb1_Size);
	fprintf(fp, "	setParameter(kb1_SizeFrom, %ff);\n", fb1_SizeFrom);
	fprintf(fp, "	setParameter(kb1_RetainSize, %ff);\n", fb1_RetainSize);
	fprintf(fp, "	setParameter(kb1_Size2Tempo, %ff);\n", fb1_Size2Tempo);
	fprintf(fp, "	setParameter(kb1_RecThreshold, %ff);\n", fb1_RecThreshold);
	fprintf(fp, "	setParameter(kb1_Speed_Pitch, %ff);\n", fb1_Speed_Pitch);
	fprintf(fp, "	setParameter(kb1_Level, %ff);\n", fb1_Level);
	fprintf(fp, "	setParameter(kb1_Input, %ff);\n", fb1_Input);
	fprintf(fp, "	setParameter(kb1_StretchFile, %ff);\n", fb1_StretchFile);
	fprintf(fp, "	setParameter(kb1_LinearInterp, %ff);\n", fb1_LinearInterp);
	fprintf(fp, "	setParameter(kb1_Reverse, %ff);\n", fb1_Reverse);
	fprintf(fp, "	setParameter(kb1_OnlyOPWhenFrozen, %ff);\n", fb1_OnlyOPWhenFrozen);
	fprintf(fp, "	setParameter(kb1_MIDINotesSetFreeze, %ff);\n", fb1_MIDINotesSetFreeze);
	fprintf(fp, "	setParameter(kb1_Freeze, %ff);\n", fb1_Freeze);
	fprintf(fp, "	setParameter(kb1_SizeLessThanMaxFreezes, %ff);\n", fb1_SizeLessThanMaxFreezes);
	fprintf(fp, "	setParameter(kb1_InvertSize, %ff);\n", fb1_InvertSize);
	fprintf(fp, "	setParameter(kb1_ReadPosition, %ff);\n", fb1_ReadPosition);
	fprintf(fp, "	setParameter(kb1_ResetRPOnMIDINote, %ff);\n", fb1_ResetRPOnMIDINote);
	fprintf(fp, "	setParameter(kb1_Pan, %ff);\n", fb1_Pan);
	fprintf(fp, "	setParameter(kb1_IPGain, %ff);\n", fb1_IPGain);

	fprintf(fp, "	setParameter(kb2_Start, %ff);\n", fb2_Start);
	fprintf(fp, "	setParameter(kb2_End, %ff);\n", fb2_End);
	fprintf(fp, "	setParameter(kb2_Size, %ff);\n", fb2_Size);
	fprintf(fp, "	setParameter(kb2_SizeFrom, %ff);\n", fb2_SizeFrom);
	fprintf(fp, "	setParameter(kb2_RetainSize, %ff);\n", fb2_RetainSize);
	fprintf(fp, "	setParameter(kb2_Size2Tempo, %ff);\n", fb2_Size2Tempo);
	fprintf(fp, "	setParameter(kb2_RecThreshold, %ff);\n", fb2_RecThreshold);
	fprintf(fp, "	setParameter(kb2_Speed_Pitch, %ff);\n", fb2_Speed_Pitch);
	fprintf(fp, "	setParameter(kb2_Level, %ff);\n", fb2_Level);
	fprintf(fp, "	setParameter(kb2_Input, %ff);\n", fb2_Input);
	fprintf(fp, "	setParameter(kb2_StretchFile, %ff);\n", fb2_StretchFile);
	fprintf(fp, "	setParameter(kb2_LinearInterp, %ff);\n", fb2_LinearInterp);
	fprintf(fp, "	setParameter(kb2_Reverse, %ff);\n", fb2_Reverse);
	fprintf(fp, "	setParameter(kb2_OnlyOPWhenFrozen, %ff);\n", fb2_OnlyOPWhenFrozen);
	fprintf(fp, "	setParameter(kb2_MIDINotesSetFreeze, %ff);\n", fb2_MIDINotesSetFreeze);
	fprintf(fp, "	setParameter(kb2_Freeze, %ff);\n", fb2_Freeze);
	fprintf(fp, "	setParameter(kb2_SizeLessThanMaxFreezes, %ff);\n", fb2_SizeLessThanMaxFreezes);
	fprintf(fp, "	setParameter(kb2_InvertSize, %ff);\n", fb2_InvertSize);
	fprintf(fp, "	setParameter(kb2_ModDestination, %ff);\n", fb2_ModDestination);
	fprintf(fp, "	setParameter(kb2_ModDepth, %ff);\n", fb2_ModDepth);
	fprintf(fp, "	setParameter(kb2_Envelope, %ff);\n", fb2_Envelope);
	fprintf(fp, "	setParameter(kb2_ReadPosition, %ff);\n", fb2_ReadPosition);
	fprintf(fp, "	setParameter(kb2_ResetRPOnMIDINote, %ff);\n", fb2_ResetRPOnMIDINote);
	fprintf(fp, "	setParameter(kb2_Pan, %ff);\n", fb2_Pan);
	fprintf(fp, "	setParameter(kb2_IPGain, %ff);\n", fb2_IPGain);

	fprintf(fp, "	setParameter(kae_OnOff, %ff);\n", fae_OnOff);
	fprintf(fp, "	setParameter(kae_Attack, %ff);\n", fae_Attack);
	fprintf(fp, "	setParameter(kae_Decay, %ff);\n", fae_Decay);
	fprintf(fp, "	setParameter(kae_Sustain, %ff);\n", fae_Sustain);
	fprintf(fp, "	setParameter(kae_Release, %ff);\n", fae_Release);
	fprintf(fp, "	setParameter(kae_SegmentTime, %ff);\n", fae_SegmentTime);
	/*fprintf(fp, "	setParameter(kae_FreezeTriggers, %ff);\n", fae_FreezeTriggers);
	fprintf(fp, "	setParameter(kae_MIDINotesTrigger, %ff);\n", fae_MIDINotesTrigger);*/

	fprintf(fp, "	setParameter(ke2_Attack, %ff);\n", fe2_Attack);
	fprintf(fp, "	setParameter(ke2_Decay, %ff);\n", fe2_Decay);
	fprintf(fp, "	setParameter(ke2_Sustain, %ff);\n", fe2_Sustain);
	fprintf(fp, "	setParameter(ke2_Release, %ff);\n", fe2_Release);
	fprintf(fp, "	setParameter(ke2_SegmentTime, %ff);\n", fe2_SegmentTime);
	fprintf(fp, "	setParameter(ke2_MIDINotesTrigger, %ff);\n", fe2_MIDINotesTrigger);
	fprintf(fp, "	setParameter(ke2_BarStartTriggers, %ff);\n", fe2_BarStartTriggers);
	fprintf(fp, "	setParameter(ke2_Destination, %ff);\n", fe2_Destination);
	fprintf(fp, "	setParameter(ke2_Direction, %ff);\n", fe2_Direction);
	fprintf(fp, "	setParameter(ke2_ModDepth, %ff);\n", fe2_ModDepth);

	fprintf(fp, "	setParameter(klfo1_Freq_Note, %ff);\n", flfo1_Freq_Note);
	fprintf(fp, "	setParameter(klfo1_TempoSync, %ff);\n", flfo1_TempoSync);
	fprintf(fp, "	setParameter(klfo1_Waveform, %ff);\n", flfo1_Waveform);
	fprintf(fp, "	setParameter(klfo1_BarStartResets, %ff);\n", flfo1_BarStartResets);
	fprintf(fp, "	setParameter(klfo1_MIDINotesReset, %ff);\n", flfo1_MIDINotesReset);
	fprintf(fp, "	setParameter(klfo1_Destination, %ff);\n", flfo1_Destination);
	fprintf(fp, "	setParameter(klfo1_ModDepth, %ff);\n", flfo1_ModDepth);

	fprintf(fp, "	setParameter(klfo2_Freq_Note, %ff);\n", flfo2_Freq_Note);
	fprintf(fp, "	setParameter(klfo2_TempoSync, %ff);\n", flfo2_TempoSync);
	fprintf(fp, "	setParameter(klfo2_Waveform, %ff);\n", flfo2_Waveform);
	fprintf(fp, "	setParameter(klfo2_BarStartResets, %ff);\n", flfo2_BarStartResets);
	fprintf(fp, "	setParameter(klfo2_MIDINotesReset, %ff);\n", flfo2_MIDINotesReset);
	fprintf(fp, "	setParameter(klfo2_Destination, %ff);\n", flfo2_Destination);
	fprintf(fp, "	setParameter(klfo2_ModDepth, %ff);\n", flfo2_ModDepth);

	fprintf(fp, "	setParameter(kfilt_OnOff, %ff);\n", ffilt_OnOff);
	fprintf(fp, "	setParameter(kfilt_Cutoff, %ff);\n", ffilt_Cutoff);
	fprintf(fp, "	setParameter(kfilt_Resonance, %ff);\n", ffilt_Resonance);
	fprintf(fp, "	setParameter(kfilt_Type, %ff);\n", ffilt_Type);

	fprintf(fp, "	setParameter(ksett_MIDILearn, %ff);\n", fsett_MIDILearn);
	fprintf(fp, "	setParameter(ksett_SynthMode, %ff);\n", fsett_SynthMode);
	fprintf(fp, "	setParameter(ksett_PolyphonicMode, %ff);\n", fsett_PolyphonicMode);
	fprintf(fp, "	setParameter(ksett_PitchCorrection, %ff);\n", fsett_PitchCorrection);
	fprintf(fp, "	setParameter(ksett_SaveBufferContents, %ff);\n", fsett_SaveBufferContents);

	fprintf(fp, "	setParameter(kop_Mix, %ff);\n", fop_Mix);
	fprintf(fp, "	setParameter(kop_Level, %ff);\n\n", fop_Level);

	getProgramName(temp);
	fprintf(fp, "	setProgramName(\"%s\");\n\n", temp);

	fprintf(fp, "	pd->presets[curProgram].b1_Start.MIDICC = %d;\n", pd->presets[curProgram].b1_Start.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_End.MIDICC = %d;\n", pd->presets[curProgram].b1_End.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Size.MIDICC = %d;\n", pd->presets[curProgram].b1_Size.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_SizeFrom.MIDICC = %d;\n", pd->presets[curProgram].b1_SizeFrom.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_RetainSize.MIDICC = %d;\n", pd->presets[curProgram].b1_RetainSize.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Size2Tempo.MIDICC = %d;\n", pd->presets[curProgram].b1_Size2Tempo.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_RecThreshold.MIDICC = %d;\n", pd->presets[curProgram].b1_RecThreshold.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Speed_Pitch.MIDICC = %d;\n", pd->presets[curProgram].b1_Speed_Pitch.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Level.MIDICC = %d;\n", pd->presets[curProgram].b1_Level.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Input.MIDICC = %d;\n", pd->presets[curProgram].b1_Input.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_StretchFile.MIDICC = %d;\n", pd->presets[curProgram].b1_StretchFile.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_LinearInterp.MIDICC = %d;\n", pd->presets[curProgram].b1_LinearInterp.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Reverse.MIDICC = %d;\n", pd->presets[curProgram].b1_Reverse.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_OnlyOPWhenFrozen.MIDICC = %d;\n", pd->presets[curProgram].b1_OnlyOPWhenFrozen.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_MIDINotesSetFreeze.MIDICC = %d;\n", pd->presets[curProgram].b1_MIDINotesSetFreeze.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Freeze.MIDICC = %d;\n", pd->presets[curProgram].b1_Freeze.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_SizeLessThanMaxFreezes.MIDICC = %d;\n", pd->presets[curProgram].b1_SizeLessThanMaxFreezes.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_InvertSize.MIDICC = %d;\n", pd->presets[curProgram].b1_InvertSize.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_ReadPosition.MIDICC = %d;\n", pd->presets[curProgram].b1_ReadPosition.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_ResetRPOnMIDINote.MIDICC = %d;\n", pd->presets[curProgram].b1_ResetRPOnMIDINote.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_Pan.MIDICC = %d;\n", pd->presets[curProgram].b1_Pan.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b1_IPGain.MIDICC = %d;\n", pd->presets[curProgram].b1_IPGain.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].b2_Start.MIDICC = %d;\n", pd->presets[curProgram].b2_Start.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_End.MIDICC = %d;\n", pd->presets[curProgram].b2_End.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Size.MIDICC = %d;\n", pd->presets[curProgram].b2_Size.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_SizeFrom.MIDICC = %d;\n", pd->presets[curProgram].b2_SizeFrom.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_RetainSize.MIDICC = %d;\n", pd->presets[curProgram].b2_RetainSize.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Size2Tempo.MIDICC = %d;\n", pd->presets[curProgram].b2_Size2Tempo.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_RecThreshold.MIDICC = %d;\n", pd->presets[curProgram].b2_RecThreshold.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Speed_Pitch.MIDICC = %d;\n", pd->presets[curProgram].b2_Speed_Pitch.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Level.MIDICC = %d;\n", pd->presets[curProgram].b2_Level.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Input.MIDICC = %d;\n", pd->presets[curProgram].b2_Input.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_StretchFile.MIDICC = %d;\n", pd->presets[curProgram].b2_StretchFile.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_LinearInterp.MIDICC = %d;\n", pd->presets[curProgram].b2_LinearInterp.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Reverse.MIDICC = %d;\n", pd->presets[curProgram].b2_Reverse.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_OnlyOPWhenFrozen.MIDICC = %d;\n", pd->presets[curProgram].b2_OnlyOPWhenFrozen.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_MIDINotesSetFreeze.MIDICC = %d;\n", pd->presets[curProgram].b2_MIDINotesSetFreeze.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Freeze.MIDICC = %d;\n", pd->presets[curProgram].b2_Freeze.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_ModDestination.MIDICC = %d;\n", pd->presets[curProgram].b2_ModDestination.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_ModDepth.MIDICC = %d;\n", pd->presets[curProgram].b2_ModDepth.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_SizeLessThanMaxFreezes.MIDICC = %d;\n", pd->presets[curProgram].b2_SizeLessThanMaxFreezes.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_InvertSize.MIDICC = %d;\n", pd->presets[curProgram].b2_InvertSize.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Envelope.MIDICC = %d;\n", pd->presets[curProgram].b2_Envelope.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_ReadPosition.MIDICC = %d;\n", pd->presets[curProgram].b2_ReadPosition.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_ResetRPOnMIDINote.MIDICC = %d;\n", pd->presets[curProgram].b2_ResetRPOnMIDINote.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_Pan.MIDICC = %d;\n", pd->presets[curProgram].b2_Pan.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].b2_IPGain.MIDICC = %d;\n", pd->presets[curProgram].b2_IPGain.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].ae_OnOff.MIDICC = %d;\n", pd->presets[curProgram].ae_OnOff.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].ae_Attack.MIDICC = %d;\n", pd->presets[curProgram].ae_Attack.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].ae_Decay.MIDICC = %d;\n", pd->presets[curProgram].ae_Decay.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].ae_Sustain.MIDICC = %d;\n", pd->presets[curProgram].ae_Sustain.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].ae_Release.MIDICC = %d;\n", pd->presets[curProgram].ae_Release.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].ae_SegmentTime.MIDICC = %d;\n", pd->presets[curProgram].ae_SegmentTime.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].e2_Attack.MIDICC = %d;\n", pd->presets[curProgram].e2_Attack.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_Decay.MIDICC = %d;\n", pd->presets[curProgram].e2_Decay.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_Sustain.MIDICC = %d;\n", pd->presets[curProgram].e2_Sustain.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_Release.MIDICC = %d;\n", pd->presets[curProgram].e2_Release.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_SegmentTime.MIDICC = %d;\n", pd->presets[curProgram].e2_SegmentTime.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_MIDINotesTrigger.MIDICC = %d;\n", pd->presets[curProgram].e2_MIDINotesTrigger.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_BarStartTriggers.MIDICC = %d;\n", pd->presets[curProgram].e2_BarStartTriggers.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_Destination.MIDICC = %d;\n", pd->presets[curProgram].e2_Destination.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_Direction.MIDICC = %d;\n", pd->presets[curProgram].e2_Direction.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].e2_ModDepth.MIDICC = %d;\n", pd->presets[curProgram].e2_ModDepth.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].lfo1_Freq_Note.MIDICC = %d;\n", pd->presets[curProgram].lfo1_Freq_Note.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo1_TempoSync.MIDICC = %d;\n", pd->presets[curProgram].lfo1_TempoSync.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo1_Waveform.MIDICC = %d;\n", pd->presets[curProgram].lfo1_Waveform.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo1_BarStartResets.MIDICC = %d;\n", pd->presets[curProgram].lfo1_BarStartResets.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo1_MIDINotesReset.MIDICC = %d;\n", pd->presets[curProgram].lfo1_MIDINotesReset.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo1_Destination.MIDICC = %d;\n", pd->presets[curProgram].lfo1_Destination.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo1_ModDepth.MIDICC = %d;\n", pd->presets[curProgram].lfo1_ModDepth.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].lfo2_Freq_Note.MIDICC = %d;\n", pd->presets[curProgram].lfo2_Freq_Note.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo2_TempoSync.MIDICC = %d;\n", pd->presets[curProgram].lfo2_TempoSync.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo2_Waveform.MIDICC = %d;\n", pd->presets[curProgram].lfo2_Waveform.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo2_BarStartResets.MIDICC = %d;\n", pd->presets[curProgram].lfo2_BarStartResets.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo2_MIDINotesReset.MIDICC = %d;\n", pd->presets[curProgram].lfo2_MIDINotesReset.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo2_Destination.MIDICC = %d;\n", pd->presets[curProgram].lfo2_Destination.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].lfo2_ModDepth.MIDICC = %d;\n", pd->presets[curProgram].lfo2_ModDepth.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].filt_OnOff.MIDICC = %d;\n", pd->presets[curProgram].filt_OnOff.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].filt_Cutoff.MIDICC = %d;\n", pd->presets[curProgram].filt_Cutoff.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].filt_Resonance.MIDICC = %d;\n", pd->presets[curProgram].filt_Resonance.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].filt_Type.MIDICC = %d;\n", pd->presets[curProgram].filt_Type.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].sett_MIDILearn.MIDICC = %d;\n", pd->presets[curProgram].sett_MIDILearn.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].sett_SynthMode.MIDICC = %d;\n", pd->presets[curProgram].sett_SynthMode.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].sett_PolyphonicMode.MIDICC = %d;\n", pd->presets[curProgram].sett_PolyphonicMode.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].sett_PitchCorrection.MIDICC = %d;\n", pd->presets[curProgram].sett_PitchCorrection.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].sett_SaveBufferContents.MIDICC = %d;\n", pd->presets[curProgram].sett_SaveBufferContents.MIDICC);

	fprintf(fp, "	pd->presets[curProgram].op_Mix.MIDICC = %d;\n", pd->presets[curProgram].op_Mix.MIDICC);
	fprintf(fp, "	pd->presets[curProgram].op_Level.MIDICC = %d;\n", pd->presets[curProgram].op_Level.MIDICC);

	fclose(fp);

	if(fsett_SaveBufferContents > 0.5f)
	{
		fp = fopen("D:\\Program Files\\Microsoft Visual Studio\\MyProjects\\VST Plugins\\Buffer Synth 2\\BS2InternalPatchBuffers.h", "w");

		NoteMaster->GetBuffer1(Buffer1[curProgram]);
		NoteMaster->GetBuffer2(Buffer2[curProgram]);

		fprintf(fp, "const float ip_Buffer1[%d] = {\n", BUFFERSIZE);
		for(i=0;i<(BUFFERSIZE-1);i++)
		{
			fprintf(fp, "	%ff,\n", Buffer1[curProgram][i]);
		}
		fprintf(fp, "	%ff\n", Buffer1[curProgram][(BUFFERSIZE-1)]);
		fprintf(fp, "};\n\n");

		fprintf(fp, "const float ip_Buffer2[%d] = {\n", BUFFERSIZE);
		for(i=0;i<(BUFFERSIZE-1);i++)
		{
			fprintf(fp, "	%ff,\n", Buffer2[curProgram][i]);
		}
		fprintf(fp, "	%ff\n", Buffer2[curProgram][(BUFFERSIZE-1)]);
		fprintf(fp, "};");

		fclose(fp);
	}
}
#endif

//----------------------------------------------------------------------------
//For processing MIDI events (among other things)
//Can ignore most of this - it's just splitting up the various kinds of MIDI signal
//(note on/off, CC etc.)
//----------------------------------------------------------------------------
long BufferSynth2::processEvents(VstEvents *ev)
{
  long note,status;
  float temp;
  double nvol;
  VstMidiEvent *event;

  for(long i = 0; (i < (ev->numEvents)); i++) //may be more than one event at one time
  { 
  if((ev->events[i])->type == kVstMidiType)   //only handle MIDI events here
  {
    event = (VstMidiEvent *)ev->events[i];     //event points to current event 
    char* midiData = event->midiData;          //pointer to MIDI data
    
    status = (midiData[0] & 0xF0);             // channel information is removed
    if((status==0x90)&&(midiData[2]>0))       // note on
    {
		note = (midiData[1] & 0x7F);            // midi note
		nvol = (midiData[2] & 0x7F)/127.0;      // velocity

		if(nvol > 0.0)
		{
			if(fb1_MIDINotesSetFreeze > 0.5f)
				setParameter(kb1_Freeze, 1.0f);
			if(fb2_MIDINotesSetFreeze > 0.5f)
				setParameter(kb2_Freeze, 1.0f);
			NoteMaster->NoteOn(note, (float)nvol);
		}
		else
		{
			if(fb1_MIDINotesSetFreeze > 0.5f)
				setParameter(kb1_Freeze, 0.0f);
			if(fb2_MIDINotesSetFreeze > 0.5f)
				setParameter(kb2_Freeze, 0.0f);
		}
    }
    else if(((status==0x90)&&(midiData[2]==0))||(status==0x80)) // note off
    {
		note = (midiData[1] & 0x7F);            // midi note
		NoteMaster->NoteOff(note);
		if(fb1_MIDINotesSetFreeze > 0.5f)
			setParameter(kb1_Freeze, 0.0f);
		if(fb2_MIDINotesSetFreeze > 0.5f)
			setParameter(kb2_Freeze, 0.0f);
    }
    else if(status==0xB0)                     // midi CC 
    {
		note = event->midiData[1];              // midi CC#
		nvol = event->midiData[2]/127.0;        // CC data
       
		if(fsett_MIDILearn < 0.5f)
		{
			if(note == pd->presets[curProgram].b1_Start.MIDICC)
				setParameter(kb1_Start, (float)nvol);
			if(note == pd->presets[curProgram].b1_End.MIDICC)
				setParameter(kb1_End, (float)nvol);
			if(note == pd->presets[curProgram].b1_Size.MIDICC)
				setParameter(kb1_Size, (float)nvol);
			if(note == pd->presets[curProgram].b1_SizeFrom.MIDICC)
				setParameter(kb1_SizeFrom, (float)nvol);
			if(note == pd->presets[curProgram].b1_RetainSize.MIDICC)
				setParameter(kb1_RetainSize, (float)nvol);
			if(note == pd->presets[curProgram].b1_Size2Tempo.MIDICC)
				setParameter(kb1_Size2Tempo, (float)nvol);
			if(note == pd->presets[curProgram].b1_RecThreshold.MIDICC)
				setParameter(kb1_RecThreshold, (float)nvol);
			if(note == pd->presets[curProgram].b1_Speed_Pitch.MIDICC)
				setParameter(kb1_Speed_Pitch, (float)nvol);
			if(note == pd->presets[curProgram].b1_Level.MIDICC)
				setParameter(kb1_Level, (float)nvol);
			if(note == pd->presets[curProgram].b1_Input.MIDICC)
				setParameter(kb1_Input, (float)nvol);
			if(note == pd->presets[curProgram].b1_StretchFile.MIDICC)
				setParameter(kb1_StretchFile, (float)nvol);
			if(note == pd->presets[curProgram].b1_LinearInterp.MIDICC)
				setParameter(kb1_LinearInterp, (float)nvol);
			if(note == pd->presets[curProgram].b1_Reverse.MIDICC)
				setParameter(kb1_Reverse, (float)nvol);
			if(note == pd->presets[curProgram].b1_OnlyOPWhenFrozen.MIDICC)
				setParameter(kb1_OnlyOPWhenFrozen, (float)nvol);
			if(note == pd->presets[curProgram].b1_MIDINotesSetFreeze.MIDICC)
				setParameter(kb1_MIDINotesSetFreeze, (float)nvol);
			if(note == pd->presets[curProgram].b1_Freeze.MIDICC)
				setParameter(kb1_Freeze, (float)nvol);
			if(note == pd->presets[curProgram].b1_SizeLessThanMaxFreezes.MIDICC)
				setParameter(kb1_SizeLessThanMaxFreezes, (float)nvol);
			if(note == pd->presets[curProgram].b1_InvertSize.MIDICC)
				setParameter(kb1_InvertSize, (float)nvol);
			if(note == pd->presets[curProgram].b1_ReadPosition.MIDICC)
				setParameter(kb1_ReadPosition, (float)nvol);
			if(note == pd->presets[curProgram].b1_ResetRPOnMIDINote.MIDICC)
				setParameter(kb1_ResetRPOnMIDINote, (float)nvol);
			if(note == pd->presets[curProgram].b1_Pan.MIDICC)
				setParameter(kb1_Pan, (float)nvol);
			if(note == pd->presets[curProgram].b1_IPGain.MIDICC)
				setParameter(kb1_IPGain, (float)nvol);
			if(note == pd->presets[curProgram].b2_Start.MIDICC)
				setParameter(kb2_Start, (float)nvol);
			if(note == pd->presets[curProgram].b2_End.MIDICC)
				setParameter(kb2_End, (float)nvol);
			if(note == pd->presets[curProgram].b2_Size.MIDICC)
				setParameter(kb2_Size, (float)nvol);
			if(note == pd->presets[curProgram].b2_SizeFrom.MIDICC)
				setParameter(kb2_SizeFrom, (float)nvol);
			if(note == pd->presets[curProgram].b2_RetainSize.MIDICC)
				setParameter(kb2_RetainSize, (float)nvol);
			if(note == pd->presets[curProgram].b2_Size2Tempo.MIDICC)
				setParameter(kb2_Size2Tempo, (float)nvol);
			if(note == pd->presets[curProgram].b2_RecThreshold.MIDICC)
				setParameter(kb2_RecThreshold, (float)nvol);
			if(note == pd->presets[curProgram].b2_Speed_Pitch.MIDICC)
				setParameter(kb2_Speed_Pitch, (float)nvol);
			if(note == pd->presets[curProgram].b2_Level.MIDICC)
				setParameter(kb2_Level, (float)nvol);
			if(note == pd->presets[curProgram].b2_Input.MIDICC)
				setParameter(kb2_Input, (float)nvol);
			if(note == pd->presets[curProgram].b2_StretchFile.MIDICC)
				setParameter(kb2_StretchFile, (float)nvol);
			if(note == pd->presets[curProgram].b2_LinearInterp.MIDICC)
				setParameter(kb2_LinearInterp, (float)nvol);
			if(note == pd->presets[curProgram].b2_Reverse.MIDICC)
				setParameter(kb2_Reverse, (float)nvol);
			if(note == pd->presets[curProgram].b2_OnlyOPWhenFrozen.MIDICC)
				setParameter(kb2_OnlyOPWhenFrozen, (float)nvol);
			if(note == pd->presets[curProgram].b2_MIDINotesSetFreeze.MIDICC)
				setParameter(kb2_MIDINotesSetFreeze, (float)nvol);
			if(note == pd->presets[curProgram].b2_Freeze.MIDICC)
				setParameter(kb2_Freeze, (float)nvol);
			if(note == pd->presets[curProgram].b2_ModDestination.MIDICC)
				setParameter(kb2_ModDestination, (float)nvol);
			if(note == pd->presets[curProgram].b2_ModDepth.MIDICC)
				setParameter(kb2_ModDepth, (float)nvol);
			if(note == pd->presets[curProgram].b2_SizeLessThanMaxFreezes.MIDICC)
				setParameter(kb2_SizeLessThanMaxFreezes, (float)nvol);
			if(note == pd->presets[curProgram].b2_InvertSize.MIDICC)
				setParameter(kb2_InvertSize, (float)nvol);
			if(note == pd->presets[curProgram].ae_OnOff.MIDICC)
				setParameter(kae_OnOff, (float)nvol);
			if(note == pd->presets[curProgram].b2_Envelope.MIDICC)
				setParameter(kb2_Envelope, (float)nvol);
			if(note == pd->presets[curProgram].b2_ReadPosition.MIDICC)
				setParameter(kb2_ReadPosition, (float)nvol);
			if(note == pd->presets[curProgram].b2_ResetRPOnMIDINote.MIDICC)
				setParameter(kb2_ResetRPOnMIDINote, (float)nvol);
			if(note == pd->presets[curProgram].b2_Pan.MIDICC)
				setParameter(kb2_Pan, (float)nvol);
			if(note == pd->presets[curProgram].b2_IPGain.MIDICC)
				setParameter(kb2_IPGain, (float)nvol);
			if(note == pd->presets[curProgram].ae_Attack.MIDICC)
				setParameter(kae_Attack, (float)nvol);
			if(note == pd->presets[curProgram].ae_Decay.MIDICC)
				setParameter(kae_Decay, (float)nvol);
			if(note == pd->presets[curProgram].ae_Sustain.MIDICC)
				setParameter(kae_Sustain, (float)nvol);
			if(note == pd->presets[curProgram].ae_Release.MIDICC)
				setParameter(kae_Release, (float)nvol);
			if(note == pd->presets[curProgram].ae_SegmentTime.MIDICC)
				setParameter(kae_SegmentTime, (float)nvol);
			/*if(note == pd->presets[curProgram].ae_FreezeTriggers.MIDICC)
				setParameter(kae_FreezeTriggers, (float)nvol);
			if(note == pd->presets[curProgram].ae_MIDINotesTrigger.MIDICC)
				setParameter(kae_MIDINotesTrigger, (float)nvol);*/
			if(note == pd->presets[curProgram].e2_Attack.MIDICC)
				setParameter(ke2_Attack, (float)nvol);
			if(note == pd->presets[curProgram].e2_Decay.MIDICC)
				setParameter(ke2_Decay, (float)nvol);
			if(note == pd->presets[curProgram].e2_Sustain.MIDICC)
				setParameter(ke2_Sustain, (float)nvol);
			if(note == pd->presets[curProgram].e2_Release.MIDICC)
				setParameter(ke2_Release, (float)nvol);
			if(note == pd->presets[curProgram].e2_SegmentTime.MIDICC)
				setParameter(ke2_SegmentTime, (float)nvol);
			if(note == pd->presets[curProgram].e2_MIDINotesTrigger.MIDICC)
				setParameter(ke2_MIDINotesTrigger, (float)nvol);
			if(note == pd->presets[curProgram].e2_BarStartTriggers.MIDICC)
				setParameter(ke2_BarStartTriggers, (float)nvol);
			if(note == pd->presets[curProgram].e2_Destination.MIDICC)
				setParameter(ke2_Destination, (float)nvol);
			if(note == pd->presets[curProgram].e2_Direction.MIDICC)
				setParameter(ke2_Direction, (float)nvol);
			if(note == pd->presets[curProgram].e2_ModDepth.MIDICC)
				setParameter(ke2_ModDepth, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_Freq_Note.MIDICC)
				setParameter(klfo1_Freq_Note, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_TempoSync.MIDICC)
				setParameter(klfo1_TempoSync, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_Waveform.MIDICC)
				setParameter(klfo1_Waveform, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_BarStartResets.MIDICC)
				setParameter(klfo1_BarStartResets, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_MIDINotesReset.MIDICC)
				setParameter(klfo1_MIDINotesReset, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_Destination.MIDICC)
				setParameter(klfo1_Destination, (float)nvol);
			if(note == pd->presets[curProgram].lfo1_ModDepth.MIDICC)
				setParameter(klfo1_ModDepth, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_Freq_Note.MIDICC)
				setParameter(klfo2_Freq_Note, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_TempoSync.MIDICC)
				setParameter(klfo2_TempoSync, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_Waveform.MIDICC)
				setParameter(klfo2_Waveform, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_BarStartResets.MIDICC)
				setParameter(klfo2_BarStartResets, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_MIDINotesReset.MIDICC)
				setParameter(klfo2_MIDINotesReset, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_Destination.MIDICC)
				setParameter(klfo2_Destination, (float)nvol);
			if(note == pd->presets[curProgram].lfo2_ModDepth.MIDICC)
				setParameter(klfo2_ModDepth, (float)nvol);
			if(note == pd->presets[curProgram].filt_OnOff.MIDICC)
				setParameter(kfilt_OnOff, (float)nvol);
			if(note == pd->presets[curProgram].filt_Cutoff.MIDICC)
				setParameter(kfilt_Cutoff, (float)nvol);
			if(note == pd->presets[curProgram].filt_Resonance.MIDICC)
				setParameter(kfilt_Resonance, (float)nvol);
			if(note == pd->presets[curProgram].filt_Type.MIDICC)
				setParameter(kfilt_Type, (float)nvol);
			if(note == pd->presets[curProgram].sett_MIDILearn.MIDICC)
				setParameter(ksett_MIDILearn, (float)nvol);
			if(note == pd->presets[curProgram].sett_SynthMode.MIDICC)
				setParameter(ksett_SynthMode, (float)nvol);
			if(note == pd->presets[curProgram].sett_PolyphonicMode.MIDICC)
				setParameter(ksett_PolyphonicMode, (float)nvol);
			if(note == pd->presets[curProgram].sett_PitchCorrection.MIDICC)
				setParameter(ksett_PitchCorrection, (float)nvol);
			if(note == pd->presets[curProgram].sett_SaveBufferContents.MIDICC)
				setParameter(ksett_SaveBufferContents, (float)nvol);
			if(note == pd->presets[curProgram].op_Mix.MIDICC)
				setParameter(kop_Mix, (float)nvol);
			if(note == pd->presets[curProgram].op_Level.MIDICC)
				setParameter(kop_Level, (float)nvol);
		}
		else if(note != 123)	//CC123 = all notes off
		{
			if(CurrentParam == kb1_Start)
			{	
				pd->presets[curProgram].b1_Start.MIDICC = note;
				setParameter(kb1_Start, (float)nvol);
			}
			else if(CurrentParam == kb1_End)
			{	
				pd->presets[curProgram].b1_End.MIDICC = note;
				setParameter(kb1_End, (float)nvol);
			}
			else if(CurrentParam == kb1_Size)
			{	
				pd->presets[curProgram].b1_Size.MIDICC = note;
				setParameter(kb1_Size, (float)nvol);
			}
			else if(CurrentParam == kb1_SizeFrom)
			{	
				pd->presets[curProgram].b1_SizeFrom.MIDICC = note;
				setParameter(kb1_SizeFrom, (float)nvol);
			}
			else if(CurrentParam == kb1_RetainSize)
			{	
				pd->presets[curProgram].b1_RetainSize.MIDICC = note;
				setParameter(kb1_RetainSize, (float)nvol);
			}
			else if(CurrentParam == kb1_Size2Tempo)
			{	
				pd->presets[curProgram].b1_Size2Tempo.MIDICC = note;
				setParameter(kb1_Size2Tempo, (float)nvol);
			}
			else if(CurrentParam == kb1_RecThreshold)
			{	
				pd->presets[curProgram].b1_RecThreshold.MIDICC = note;
				setParameter(kb1_RecThreshold, (float)nvol);
			}
			else if(CurrentParam == kb1_Speed_Pitch)
			{	
				pd->presets[curProgram].b1_Speed_Pitch.MIDICC = note;
				setParameter(kb1_Speed_Pitch, (float)nvol);
			}
			else if(CurrentParam == kb1_Level)
			{	
				pd->presets[curProgram].b1_Level.MIDICC = note;
				setParameter(kb1_Level, (float)nvol);
			}
			else if(CurrentParam == kb1_Input)
			{	
				pd->presets[curProgram].b1_Input.MIDICC = note;
				setParameter(kb1_Input, (float)nvol);
			}
			else if(CurrentParam == kb1_StretchFile)
			{	
				pd->presets[curProgram].b1_StretchFile.MIDICC = note;
				setParameter(kb1_StretchFile, (float)nvol);
			}
			else if(CurrentParam == kb1_LinearInterp)
			{	
				pd->presets[curProgram].b1_LinearInterp.MIDICC = note;
				setParameter(kb1_LinearInterp, (float)nvol);
			}
			else if(CurrentParam == kb1_Reverse)
			{	
				pd->presets[curProgram].b1_Reverse.MIDICC = note;
				setParameter(kb1_Reverse, (float)nvol);
			}
			else if(CurrentParam == kb1_OnlyOPWhenFrozen)
			{	
				pd->presets[curProgram].b1_OnlyOPWhenFrozen.MIDICC = note;
				setParameter(kb1_OnlyOPWhenFrozen, (float)nvol);
			}
			else if(CurrentParam == kb1_MIDINotesSetFreeze)
			{	
				pd->presets[curProgram].b1_MIDINotesSetFreeze.MIDICC = note;
				setParameter(kb1_MIDINotesSetFreeze, (float)nvol);
			}
			else if(CurrentParam == kb1_Freeze)
			{	
				pd->presets[curProgram].b1_Freeze.MIDICC = note;
				setParameter(kb1_Freeze, (float)nvol);
			}
			else if(CurrentParam == kb1_SizeLessThanMaxFreezes)
			{	
				pd->presets[curProgram].b1_SizeLessThanMaxFreezes.MIDICC = note;
				setParameter(kb1_SizeLessThanMaxFreezes, (float)nvol);
			}
			else if(CurrentParam == kb1_InvertSize)
			{	
				pd->presets[curProgram].b1_InvertSize.MIDICC = note;
				setParameter(kb1_InvertSize, (float)nvol);
			}
			else if(CurrentParam == kb1_ReadPosition)
			{	
				pd->presets[curProgram].b1_ReadPosition.MIDICC = note;
				setParameter(kb1_ReadPosition, (float)nvol);
			}
			else if(CurrentParam == kb1_ResetRPOnMIDINote)
			{	
				pd->presets[curProgram].b1_ResetRPOnMIDINote.MIDICC = note;
				setParameter(kb1_ResetRPOnMIDINote, (float)nvol);
			}
			else if(CurrentParam == kb1_Pan)
			{	
				pd->presets[curProgram].b1_Pan.MIDICC = note;
				setParameter(kb1_Pan, (float)nvol);
			}
			else if(CurrentParam == kb1_IPGain)
			{	
				pd->presets[curProgram].b1_IPGain.MIDICC = note;
				setParameter(kb1_IPGain, (float)nvol);
			}
			else if(CurrentParam == kb2_Start)
			{	
				pd->presets[curProgram].b2_Start.MIDICC = note;
				setParameter(kb2_Start, (float)nvol);
			}
			else if(CurrentParam == kb2_End)
			{	
				pd->presets[curProgram].b2_End.MIDICC = note;
				setParameter(kb2_End, (float)nvol);
			}
			else if(CurrentParam == kb2_Size)
			{	
				pd->presets[curProgram].b2_Size.MIDICC = note;
				setParameter(kb2_Size, (float)nvol);
			}
			else if(CurrentParam == kb2_SizeFrom)
			{	
				pd->presets[curProgram].b2_SizeFrom.MIDICC = note;
				setParameter(kb2_SizeFrom, (float)nvol);
			}
			else if(CurrentParam == kb2_RetainSize)
			{	
				pd->presets[curProgram].b2_RetainSize.MIDICC = note;
				setParameter(kb2_RetainSize, (float)nvol);
			}
			else if(CurrentParam == kb2_Size2Tempo)
			{	
				pd->presets[curProgram].b2_Size2Tempo.MIDICC = note;
				setParameter(kb2_Size2Tempo, (float)nvol);
			}
			else if(CurrentParam == kb2_RecThreshold)
			{	
				pd->presets[curProgram].b2_RecThreshold.MIDICC = note;
				setParameter(kb2_RecThreshold, (float)nvol);
			}
			else if(CurrentParam == kb2_Speed_Pitch)
			{	
				pd->presets[curProgram].b2_Speed_Pitch.MIDICC = note;
				setParameter(kb2_Speed_Pitch, (float)nvol);
			}
			else if(CurrentParam == kb2_Level)
			{	
				pd->presets[curProgram].b2_Level.MIDICC = note;
				setParameter(kb2_Level, (float)nvol);
			}
			else if(CurrentParam == kb2_Input)
			{	
				pd->presets[curProgram].b2_Input.MIDICC = note;
				setParameter(kb2_Input, (float)nvol);
			}
			else if(CurrentParam == kb2_StretchFile)
			{	
				pd->presets[curProgram].b2_StretchFile.MIDICC = note;
				setParameter(kb2_StretchFile, (float)nvol);
			}
			else if(CurrentParam == kb2_LinearInterp)
			{	
				pd->presets[curProgram].b2_LinearInterp.MIDICC = note;
				setParameter(kb2_LinearInterp, (float)nvol);
			}
			else if(CurrentParam == kb2_Reverse)
			{	
				pd->presets[curProgram].b2_Reverse.MIDICC = note;
				setParameter(kb2_Reverse, (float)nvol);
			}
			else if(CurrentParam == kb2_OnlyOPWhenFrozen)
			{	
				pd->presets[curProgram].b2_OnlyOPWhenFrozen.MIDICC = note;
				setParameter(kb2_OnlyOPWhenFrozen, (float)nvol);
			}
			else if(CurrentParam == kb2_MIDINotesSetFreeze)
			{	
				pd->presets[curProgram].b2_MIDINotesSetFreeze.MIDICC = note;
				setParameter(kb2_MIDINotesSetFreeze, (float)nvol);
			}
			else if(CurrentParam == kb2_Freeze)
			{	
				pd->presets[curProgram].b2_Freeze.MIDICC = note;
				setParameter(kb2_Freeze, (float)nvol);
			}
			else if(CurrentParam == kb2_SizeLessThanMaxFreezes)
			{	
				pd->presets[curProgram].b2_SizeLessThanMaxFreezes.MIDICC = note;
				setParameter(kb2_SizeLessThanMaxFreezes, (float)nvol);
			}
			else if(CurrentParam == kb2_InvertSize)
			{	
				pd->presets[curProgram].b2_InvertSize.MIDICC = note;
				setParameter(kb2_InvertSize, (float)nvol);
			}
			else if(CurrentParam == kb2_ModDestination)
			{	
				pd->presets[curProgram].b2_ModDestination.MIDICC = note;
				setParameter(kb2_ModDestination, (float)nvol);
			}
			else if(CurrentParam == kb2_ModDepth)
			{	
				pd->presets[curProgram].b2_ModDepth.MIDICC = note;
				setParameter(kb2_ModDepth, (float)nvol);
			}
			else if(CurrentParam == kb2_Envelope)
			{	
				pd->presets[curProgram].b2_Envelope.MIDICC = note;
				setParameter(kb2_Envelope, (float)nvol);
			}
			else if(CurrentParam == kb2_ReadPosition)
			{	
				pd->presets[curProgram].b2_ReadPosition.MIDICC = note;
				setParameter(kb2_ReadPosition, (float)nvol);
			}
			else if(CurrentParam == kb2_ResetRPOnMIDINote)
			{	
				pd->presets[curProgram].b2_ResetRPOnMIDINote.MIDICC = note;
				setParameter(kb2_ResetRPOnMIDINote, (float)nvol);
			}
			else if(CurrentParam == kb2_Pan)
			{	
				pd->presets[curProgram].b2_Pan.MIDICC = note;
				setParameter(kb2_Pan, (float)nvol);
			}
			else if(CurrentParam == kb2_IPGain)
			{	
				pd->presets[curProgram].b2_IPGain.MIDICC = note;
				setParameter(kb2_IPGain, (float)nvol);
			}
			else if(CurrentParam == kae_OnOff)
			{	
				pd->presets[curProgram].ae_OnOff.MIDICC = note;
				setParameter(kae_OnOff, (float)nvol);
			}
			else if(CurrentParam == kae_Attack)
			{	
				pd->presets[curProgram].ae_Attack.MIDICC = note;
				setParameter(kae_Attack, (float)nvol);
			}
			else if(CurrentParam == kae_Decay)
			{	
				pd->presets[curProgram].ae_Decay.MIDICC = note;
				setParameter(kae_Decay, (float)nvol);
			}
			else if(CurrentParam == kae_Sustain)
			{	
				pd->presets[curProgram].ae_Sustain.MIDICC = note;
				setParameter(kae_Sustain, (float)nvol);
			}
			else if(CurrentParam == kae_Release)
			{	
				pd->presets[curProgram].ae_Release.MIDICC = note;
				setParameter(kae_Release, (float)nvol);
			}
			else if(CurrentParam == kae_SegmentTime)
			{	
				pd->presets[curProgram].ae_SegmentTime.MIDICC = note;
				setParameter(kae_SegmentTime, (float)nvol);
			}
			/*else if(CurrentParam == kae_FreezeTriggers)
			{	
				pd->presets[curProgram].ae_FreezeTriggers.MIDICC = note;
				setParameter(kae_FreezeTriggers, (float)nvol);
			}
			else if(CurrentParam == kae_MIDINotesTrigger)
			{	
				pd->presets[curProgram].ae_MIDINotesTrigger.MIDICC = note;
				setParameter(kae_MIDINotesTrigger, (float)nvol);
			}*/
			else if(CurrentParam == ke2_Attack)
			{	
				pd->presets[curProgram].e2_Attack.MIDICC = note;
				setParameter(ke2_Attack, (float)nvol);
			}
			else if(CurrentParam == ke2_Decay)
			{	
				pd->presets[curProgram].e2_Decay.MIDICC = note;
				setParameter(ke2_Decay, (float)nvol);
			}
			else if(CurrentParam == ke2_Sustain)
			{	
				pd->presets[curProgram].e2_Sustain.MIDICC = note;
				setParameter(ke2_Sustain, (float)nvol);
			}
			else if(CurrentParam == ke2_Release)
			{	
				pd->presets[curProgram].e2_Release.MIDICC = note;
				setParameter(ke2_Release, (float)nvol);
			}
			else if(CurrentParam == ke2_SegmentTime)
			{	
				pd->presets[curProgram].e2_SegmentTime.MIDICC = note;
				setParameter(ke2_SegmentTime, (float)nvol);
			}
			else if(CurrentParam == ke2_MIDINotesTrigger)
			{	
				pd->presets[curProgram].e2_MIDINotesTrigger.MIDICC = note;
				setParameter(ke2_MIDINotesTrigger, (float)nvol);
			}
			else if(CurrentParam == ke2_BarStartTriggers)
			{	
				pd->presets[curProgram].e2_BarStartTriggers.MIDICC = note;
				setParameter(ke2_BarStartTriggers, (float)nvol);
			}
			else if(CurrentParam == ke2_Destination)
			{	
				pd->presets[curProgram].e2_Destination.MIDICC = note;
				setParameter(ke2_Destination, (float)nvol);
			}
			else if(CurrentParam == ke2_Direction)
			{	
				pd->presets[curProgram].e2_Direction.MIDICC = note;
				setParameter(ke2_Direction, (float)nvol);
			}
			else if(CurrentParam == ke2_ModDepth)
			{	
				pd->presets[curProgram].e2_ModDepth.MIDICC = note;
				setParameter(ke2_ModDepth, (float)nvol);
			}
			else if(CurrentParam == klfo1_Freq_Note)
			{	
				pd->presets[curProgram].lfo1_Freq_Note.MIDICC = note;
				setParameter(klfo1_Freq_Note, (float)nvol);
			}
			else if(CurrentParam == klfo1_TempoSync)
			{	
				pd->presets[curProgram].lfo1_TempoSync.MIDICC = note;
				setParameter(klfo1_TempoSync, (float)nvol);
			}
			else if(CurrentParam == klfo1_Waveform)
			{	
				pd->presets[curProgram].lfo1_Waveform.MIDICC = note;
				setParameter(klfo1_Waveform, (float)nvol);
			}
			else if(CurrentParam == klfo1_BarStartResets)
			{	
				pd->presets[curProgram].lfo1_BarStartResets.MIDICC = note;
				setParameter(klfo1_BarStartResets, (float)nvol);
			}
			else if(CurrentParam == klfo1_MIDINotesReset)
			{	
				pd->presets[curProgram].lfo1_MIDINotesReset.MIDICC = note;
				setParameter(klfo1_MIDINotesReset, (float)nvol);
			}
			else if(CurrentParam == klfo1_Destination)
			{	
				pd->presets[curProgram].lfo1_Destination.MIDICC = note;
				setParameter(klfo1_Destination, (float)nvol);
			}
			else if(CurrentParam == klfo1_ModDepth)
			{	
				pd->presets[curProgram].lfo1_ModDepth.MIDICC = note;
				setParameter(klfo1_ModDepth, (float)nvol);
			}
			else if(CurrentParam == klfo2_Freq_Note)
			{	
				pd->presets[curProgram].lfo2_Freq_Note.MIDICC = note;
				setParameter(klfo2_Freq_Note, (float)nvol);
			}
			else if(CurrentParam == klfo2_TempoSync)
			{	
				pd->presets[curProgram].lfo2_TempoSync.MIDICC = note;
				setParameter(klfo2_TempoSync, (float)nvol);
			}
			else if(CurrentParam == klfo2_Waveform)
			{	
				pd->presets[curProgram].lfo2_Waveform.MIDICC = note;
				setParameter(klfo2_Waveform, (float)nvol);
			}
			else if(CurrentParam == klfo2_BarStartResets)
			{	
				pd->presets[curProgram].lfo2_BarStartResets.MIDICC = note;
				setParameter(klfo2_BarStartResets, (float)nvol);
			}
			else if(CurrentParam == klfo2_MIDINotesReset)
			{	
				pd->presets[curProgram].lfo2_MIDINotesReset.MIDICC = note;
				setParameter(klfo2_MIDINotesReset, (float)nvol);
			}
			else if(CurrentParam == klfo2_Destination)
			{	
				pd->presets[curProgram].lfo2_Destination.MIDICC = note;
				setParameter(klfo2_Destination, (float)nvol);
			}
			else if(CurrentParam == klfo2_ModDepth)
			{	
				pd->presets[curProgram].lfo2_ModDepth.MIDICC = note;
				setParameter(klfo2_ModDepth, (float)nvol);
			}
			else if(CurrentParam == kfilt_OnOff)
			{	
				pd->presets[curProgram].filt_OnOff.MIDICC = note;
				setParameter(kfilt_OnOff, (float)nvol);
			}
			else if(CurrentParam == kfilt_Cutoff)
			{	
				pd->presets[curProgram].filt_Cutoff.MIDICC = note;
				setParameter(kfilt_Cutoff, (float)nvol);
			}
			else if(CurrentParam == kfilt_Resonance)
			{	
				pd->presets[curProgram].filt_Resonance.MIDICC = note;
				setParameter(kfilt_Resonance, (float)nvol);
			}
			else if(CurrentParam == kfilt_Type)
			{	
				pd->presets[curProgram].filt_Type.MIDICC = note;
				setParameter(kfilt_Type, (float)nvol);
			}
			else if(CurrentParam == ksett_MIDILearn)
			{	
				pd->presets[curProgram].sett_MIDILearn.MIDICC = note;
				setParameter(ksett_MIDILearn, (float)nvol);
			}
			else if(CurrentParam == ksett_SynthMode)
			{	
				pd->presets[curProgram].sett_SynthMode.MIDICC = note;
				setParameter(ksett_SynthMode, (float)nvol);
			}
			else if(CurrentParam == ksett_PolyphonicMode)
			{	
				pd->presets[curProgram].sett_PolyphonicMode.MIDICC = note;
				setParameter(ksett_PolyphonicMode, (float)nvol);
			}
			else if(CurrentParam == ksett_PitchCorrection)
			{	
				pd->presets[curProgram].sett_PitchCorrection.MIDICC = note;
				setParameter(ksett_PitchCorrection, (float)nvol);
			}
			else if(CurrentParam == ksett_SaveBufferContents)
			{	
				pd->presets[curProgram].sett_SaveBufferContents.MIDICC = note;
				setParameter(ksett_SaveBufferContents, (float)nvol);
			}
			else if(CurrentParam == kop_Mix)
			{	
				pd->presets[curProgram].op_Mix.MIDICC = note;
				setParameter(kop_Mix, (float)nvol);
			}
			else if(CurrentParam == kop_Level)
			{	
				pd->presets[curProgram].op_Level.MIDICC = note;
				setParameter(kop_Level, (float)nvol);
			}
		}
    }
    else if(status==0xE0)                     // pitchbend 
    {
        temp = (float)((event->midiData[2]) * 256);         //MSB
        nvol = (float)((temp + (event->midiData[1]))/65536);//LSB - pitchbend data (2 bytes worth)
        
        if (editor)
              editor->postUpdate();
    }
	else if(status==0xC0)	//program change
	{
		note = (long)event->midiData[1] & 0x7F;
		if(note < kNumPrograms)
		{
			setProgram(note);
		}
	}
  }
}
return 1;
}

//----------------------------------------------------------------------------
bool BufferSynth2::getBarStart()
{
	unsigned long bs, ppq;
	//double bs, ppq;
	//unsigned long temp;
	//double temp;

	//checkTooSoon = 0;
	if(checkTooSoon > 0)
	{
		checkTooSoon--;
		return false;
	}


	VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid);
	if(!TimeInfo)
		return false;

	bs = (unsigned long)((double)TimeInfo->barStartPos*100.0);
	ppq = (unsigned long)((double)TimeInfo->ppqPos*100.0);

	if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
		return false;
	if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
		return false;

	if(bs != 0)
	{
		checkBSZero = false;
		//temp = ppq % bs;
		//temp = fmod(ppq, bs);
		if(ppq == bs)
		{
			//checkTooSoon = 2048;
			return true;
		}
		else
			return false;
	}
	else if(checkBSZero == false)
	{
		checkBSZero = true;
		if(ppq == 0)
		{
			//checkTooSoon = 2048;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

//----------------------------------------------------------------------------
void BufferSynth2::resume()
{
    wantEvents(); // important for all plugins that receive MIDI! 

	tempo = (float)((float)tempoAt(0)*0.0001f);
	if(tempo <= 0.0f)
		tempo = 125.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;
	NoteMaster->SetSamplerate(samplerate);
}

//----------------------------------------------------------------------------
void BufferSynth2::suspend()
{
    // buffers are normally cleared/initialized here
}

//--------------------------------------------------------------------------
long BufferSynth2::getChunk (void **data, bool isPreset)
{
	int i;
	for(i=0;i<kNumPrograms;i++)
	{
		if(pd->presets[i].sett_SaveBufferContents.val < 0.5f)
			strcpy(pd->presets[i].patchPath, "");
	}
    if(isPreset) //true = patch, false = bank
    {
        *data = &pd->presets[curProgram];
        return sizeof(BS2Preset);
    }
    *data = pd;
    return sizeof(BS2PresetData);
}

//--------------------------------------------------------------------------
long BufferSynth2::setChunk (void *data, long byteSize, bool isPreset)
{
	//int i;
	//float *temp;

	//temp = NULL;

    if(isPreset)
    {
        if (byteSize != sizeof(BS2Preset))
            return 0;

		memcpy(&pd->presets[curProgram], (BS2Preset*)data, sizeof(BS2Preset));
    }
    else
    {
        if (byteSize != sizeof(BS2PresetData))
            return 0;

		memset(pd, 0, sizeof(BS2PresetData));
		memcpy(pd, (BS2PresetData *)data, sizeof(BS2PresetData));
    }
	setProgram(curProgram);

	//And load the .bsp file if we're supposed to.
	if(strcmp(pd->presets[curProgram].patchPath, "")!=0)
		LoadPatch(pd->presets[curProgram].patchPath);

    return 1;
}

//----------------------------------------------------------------------------
void BufferSynth2::setProgram(long program)
{
    //BufferSynth2Prog *ap = &programs[program];

	NoteMaster->GetBuffer1(Buffer1[curProgram], BUFFERSIZE);	//i.e. we save the contents of the current buffers before we switch patches
	NoteMaster->GetBuffer2(Buffer2[curProgram], BUFFERSIZE);

	if(program >= kNumPrograms)
		curProgram = 0;
	else
		curProgram = program;
	//buffer 1
	//setParameter(kb1_Start,					pd->presets[curProgram].b1_Start.val);
	fb1_Start =									pd->presets[curProgram].b1_Start.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb1_Start, fb1_Start);

	//setParameter(kb1_End,					pd->presets[curProgram].b1_End.val);
	fb1_End =									pd->presets[curProgram].b1_End.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb1_End, fb1_End);

	//setParameter(kb1_Size,					pd->presets[curProgram].b1_Size.val);
	fb1_Size =								pd->presets[curProgram].b1_Size.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb1_Size, fb1_Size);

	setParameter(kb1_SizeFrom,				pd->presets[curProgram].b1_SizeFrom.val);
	setParameter(kb1_RetainSize,			pd->presets[curProgram].b1_RetainSize.val);
	setParameter(kb1_Size2Tempo,			pd->presets[curProgram].b1_Size2Tempo.val);
	setParameter(kb1_RecThreshold,			pd->presets[curProgram].b1_RecThreshold.val);
	setParameter(kb1_Speed_Pitch,			pd->presets[curProgram].b1_Speed_Pitch.val);
	setParameter(kb1_Level,					pd->presets[curProgram].b1_Level.val);
	setParameter(kb1_Input,					pd->presets[curProgram].b1_Input.val);
	setParameter(kb1_StretchFile,			pd->presets[curProgram].b1_StretchFile.val);
	setParameter(kb1_LinearInterp,			pd->presets[curProgram].b1_LinearInterp.val);
	setParameter(kb1_Reverse,				pd->presets[curProgram].b1_Reverse.val);
	setParameter(kb1_OnlyOPWhenFrozen,		pd->presets[curProgram].b1_OnlyOPWhenFrozen.val);
	setParameter(kb1_MIDINotesSetFreeze,	pd->presets[curProgram].b1_MIDINotesSetFreeze.val);
	setParameter(kb1_Freeze,				pd->presets[curProgram].b1_Freeze.val);
	setParameter(kb1_SizeLessThanMaxFreezes,pd->presets[curProgram].b1_SizeLessThanMaxFreezes.val);
	//setParameter(kb1_InvertSize,			pd->presets[curProgram].b1_InvertSize.val);
	fb1_InvertSize =						pd->presets[curProgram].b1_InvertSize.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb1_InvertSize, fb1_InvertSize);

	setParameter(kb1_ReadPosition,			pd->presets[curProgram].b1_ReadPosition.val);
	setParameter(kb1_ResetRPOnMIDINote,		pd->presets[curProgram].b1_ResetRPOnMIDINote.val);
	setParameter(kb1_Pan,					pd->presets[curProgram].b1_Pan.val);
	setParameter(kb1_IPGain,				pd->presets[curProgram].b1_IPGain.val);

	//buffer 2
	//setParameter(kb2_Start,					pd->presets[curProgram].b2_Start.val);
	fb2_Start =									pd->presets[curProgram].b1_Start.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb2_Start, fb2_Start);

	//setParameter(kb2_End,					pd->presets[curProgram].b2_End.val);
	fb2_End =									pd->presets[curProgram].b2_End.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb2_End, fb2_End);

	//setParameter(kb2_Size,					pd->presets[curProgram].b2_Size.val);
	fb2_Size =								pd->presets[curProgram].b2_Size.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb2_Size, fb2_Size);

	setParameter(kb2_End,					pd->presets[curProgram].b2_End.val);
	setParameter(kb2_Size,					pd->presets[curProgram].b2_Size.val);
	setParameter(kb2_SizeFrom,				pd->presets[curProgram].b2_SizeFrom.val);
	setParameter(kb2_RetainSize,			pd->presets[curProgram].b2_RetainSize.val);
	setParameter(kb2_Size2Tempo,			pd->presets[curProgram].b2_Size2Tempo.val);
	setParameter(kb2_RecThreshold,			pd->presets[curProgram].b2_RecThreshold.val);
	setParameter(kb2_Speed_Pitch,			pd->presets[curProgram].b2_Speed_Pitch.val);
	setParameter(kb2_Level,					pd->presets[curProgram].b2_Level.val);
	setParameter(kb2_Input,					pd->presets[curProgram].b2_Input.val);
	setParameter(kb2_StretchFile,			pd->presets[curProgram].b2_StretchFile.val);
	setParameter(kb2_LinearInterp,			pd->presets[curProgram].b2_LinearInterp.val);
	setParameter(kb2_Reverse,				pd->presets[curProgram].b2_Reverse.val);
	setParameter(kb2_OnlyOPWhenFrozen,		pd->presets[curProgram].b2_OnlyOPWhenFrozen.val);
	setParameter(kb2_MIDINotesSetFreeze,	pd->presets[curProgram].b2_MIDINotesSetFreeze.val);
	setParameter(kb2_Freeze,				pd->presets[curProgram].b2_Freeze.val);
	setParameter(kb2_ModDestination,		pd->presets[curProgram].b2_ModDestination.val);
	setParameter(kb2_ModDepth,				pd->presets[curProgram].b2_ModDepth.val);
	setParameter(kb2_SizeLessThanMaxFreezes,pd->presets[curProgram].b2_SizeLessThanMaxFreezes.val);
	//setParameter(kb2_InvertSize,			pd->presets[curProgram].b2_InvertSize.val);
	fb2_InvertSize =						pd->presets[curProgram].b2_InvertSize.val;
	if(NoteMaster)
		NoteMaster->SetParameter(kb2_InvertSize, fb2_InvertSize);

	setParameter(kb2_Envelope,				pd->presets[curProgram].b2_Envelope.val);
	setParameter(kb2_ReadPosition,			pd->presets[curProgram].b2_ReadPosition.val);
	setParameter(kb2_ResetRPOnMIDINote,		pd->presets[curProgram].b2_ResetRPOnMIDINote.val);
	setParameter(kb2_Pan,					pd->presets[curProgram].b2_Pan.val);
	setParameter(kb2_IPGain,				pd->presets[curProgram].b2_IPGain.val);

	//amplitude envelope
	setParameter(kae_OnOff,					pd->presets[curProgram].ae_OnOff.val);
	setParameter(kae_Attack,				pd->presets[curProgram].ae_Attack.val);
	setParameter(kae_Decay,					pd->presets[curProgram].ae_Decay.val);
	setParameter(kae_Sustain,				pd->presets[curProgram].ae_Sustain.val);
	setParameter(kae_Release,				pd->presets[curProgram].ae_Release.val);
	setParameter(kae_SegmentTime,			pd->presets[curProgram].ae_SegmentTime.val);
	/*setParameter(kae_FreezeTriggers,		pd->presets[curProgram].ae_FreezeTriggers.val);
	setParameter(kae_MIDINotesTrigger,		pd->presets[curProgram].ae_MIDINotesTrigger.val);*/

	//second envelope
	setParameter(ke2_Attack,				pd->presets[curProgram].e2_Attack.val);
	setParameter(ke2_Decay,					pd->presets[curProgram].e2_Decay.val);
	setParameter(ke2_Sustain,				pd->presets[curProgram].e2_Sustain.val);
	setParameter(ke2_Release,				pd->presets[curProgram].e2_Release.val);
	setParameter(ke2_SegmentTime,			pd->presets[curProgram].e2_SegmentTime.val);
	setParameter(ke2_MIDINotesTrigger,		pd->presets[curProgram].e2_MIDINotesTrigger.val);
	setParameter(ke2_BarStartTriggers,		pd->presets[curProgram].e2_BarStartTriggers.val);
	setParameter(ke2_Destination,			pd->presets[curProgram].e2_Destination.val);
	setParameter(ke2_Direction,				pd->presets[curProgram].e2_Direction.val);
	setParameter(ke2_ModDepth,				pd->presets[curProgram].e2_ModDepth.val);

	//LFO1
	setParameter(klfo1_TempoSync,			pd->presets[curProgram].lfo1_TempoSync.val); //do this first, because Freq_Note relies on it...
	setParameter(klfo1_Freq_Note,			pd->presets[curProgram].lfo1_Freq_Note.val);
	setParameter(klfo1_Waveform,			pd->presets[curProgram].lfo1_Waveform.val);
	setParameter(klfo1_BarStartResets,		pd->presets[curProgram].lfo1_BarStartResets.val);
	setParameter(klfo1_MIDINotesReset,		pd->presets[curProgram].lfo1_MIDINotesReset.val);
	setParameter(klfo1_Destination,			pd->presets[curProgram].lfo1_Destination.val);
	setParameter(klfo1_ModDepth,			pd->presets[curProgram].lfo1_ModDepth.val);

	//LFO2
	setParameter(klfo2_TempoSync,			pd->presets[curProgram].lfo2_TempoSync.val);
	setParameter(klfo2_Freq_Note,			pd->presets[curProgram].lfo2_Freq_Note.val);
	setParameter(klfo2_Waveform,			pd->presets[curProgram].lfo2_Waveform.val);
	setParameter(klfo2_BarStartResets,		pd->presets[curProgram].lfo2_BarStartResets.val);
	setParameter(klfo2_MIDINotesReset,		pd->presets[curProgram].lfo2_MIDINotesReset.val);
	setParameter(klfo2_Destination,			pd->presets[curProgram].lfo2_Destination.val);
	setParameter(klfo2_ModDepth,			pd->presets[curProgram].lfo2_ModDepth.val);

	//filter
	setParameter(kfilt_OnOff,				pd->presets[curProgram].filt_OnOff.val);
	setParameter(kfilt_Cutoff,				pd->presets[curProgram].filt_Cutoff.val);
	setParameter(kfilt_Resonance,			pd->presets[curProgram].filt_Resonance.val);
	setParameter(kfilt_Type,				pd->presets[curProgram].filt_Type.val);

	//settings
	setParameter(ksett_MIDILearn,			pd->presets[curProgram].sett_MIDILearn.val);
	setParameter(ksett_SynthMode,			pd->presets[curProgram].sett_SynthMode.val);
	setParameter(ksett_PolyphonicMode,		pd->presets[curProgram].sett_PolyphonicMode.val);
	setParameter(ksett_PitchCorrection,		pd->presets[curProgram].sett_PitchCorrection.val);
	setParameter(ksett_SaveBufferContents,	pd->presets[curProgram].sett_SaveBufferContents.val);

	//output
	setParameter(kop_Mix,					pd->presets[curProgram].op_Mix.val);
	setParameter(kop_Level,					pd->presets[curProgram].op_Level.val);

	if(NoteMaster)
	{
		NoteMaster->Write2IPBuffer1(Buffer1[curProgram]);
		NoteMaster->Write2IPBuffer2(Buffer2[curProgram]);
	}
}

//----------------------------------------------------------------------------
void BufferSynth2::setProgramName (char *name)
{
	strcpy (pd->presets[curProgram].name, name);
}


//----------------------------------------------------------------------------
void BufferSynth2::getProgramName(char *name)
{
	/*if(!strcmp(pd->presets[curProgram].name, "Init"))
		sprintf(name, "%s %d", pd->presets[curProgram].name, curProgram + 1);
	else*/
		strcpy(name, pd->presets[curProgram].name);
}

//----------------------------------------------------------------------------
bool BufferSynth2::getProgramNameIndexed (long category, long index, char* text)
{
    if (index<kNumPrograms)
    {
      strcpy(text,pd->presets[index].name);
      return true;
    }
    return false;
}

//----------------------------------------------------------------------------
bool BufferSynth2::copyProgram (long destination)
{
	bool returnCode = false;
    if(destination < kNumPrograms)
    {        pd->presets[destination] = pd->presets[curProgram];
        returnCode = true;
    }
    return (returnCode);
}

//----------------------------------------------------------------------------
void BufferSynth2::setSize1FromStartEnd(float val)
{
	fb1_Size = pd->presets[curProgram].b1_Size.val = val;
	if(fb1_InvertSize < 0.5f)
	{
		if((fb1_SizeLessThanMaxFreezes > 0.5f)&&(fb1_Size < 1.0f))
			setParameter(kb1_Freeze, 1.0f);
		else if(fb1_SizeLessThanMaxFreezes > 0.5f)
			setParameter(kb1_Freeze, 0.0f);
	}
	else
	{
		if((fb1_SizeLessThanMaxFreezes > 0.5f)&&((1.0f-fb1_Size) < 1.0f))
			setParameter(kb1_Freeze, 1.0f);
		else if(fb1_SizeLessThanMaxFreezes > 0.5f)
			setParameter(kb1_Freeze, 0.0f);
	}

	if(NoteMaster)
		NoteMaster->SetParameter(kb1_Size, fb1_Size);
}

//----------------------------------------------------------------------------
void BufferSynth2::setStart1FromEnd(float val)
{
	if(val < 0.0f)
		val = 0.0f;
	fb1_Start = pd->presets[curProgram].b1_Start.val = val;

	if(NoteMaster)
		NoteMaster->SetParameter(kb1_Start, fb1_Start);
}

//----------------------------------------------------------------------------
void BufferSynth2::setEnd1FromStart(float val)
{
	if(val > 1.0f)
		val = 1.0f;
	fb1_End = pd->presets[curProgram].b1_End.val = val;

	if(NoteMaster)
		NoteMaster->SetParameter(kb1_End, fb1_End);
}

//----------------------------------------------------------------------------
void BufferSynth2::setSize2FromStartEnd(float val)
{
	fb2_Size = pd->presets[curProgram].b2_Size.val = val;
	if(fb2_InvertSize < 0.5f)
	{
		if((fb2_SizeLessThanMaxFreezes > 0.5f)&&(fb2_Size < 1.0f))
			setParameter(kb2_Freeze, 1.0f);
		else if(fb2_SizeLessThanMaxFreezes > 0.5f)
			setParameter(kb2_Freeze, 0.0f);
	}
	else
	{
		if((fb2_SizeLessThanMaxFreezes > 0.5f)&&((1.0f-fb2_Size) < 1.0f))
			setParameter(kb2_Freeze, 1.0f);
		else if(fb2_SizeLessThanMaxFreezes > 0.5f)
			setParameter(kb2_Freeze, 0.0f);
	}

	if(NoteMaster)
		NoteMaster->SetParameter(kb2_Size, fb2_Size);
}

//----------------------------------------------------------------------------
void BufferSynth2::setStart2FromEnd(float val)
{
	if(val < 0.0f)
		val = 0.0f;
	fb2_Start = pd->presets[curProgram].b2_Start.val = val;

	if(NoteMaster)
		NoteMaster->SetParameter(kb2_Start, fb2_Start);
}

//----------------------------------------------------------------------------
void BufferSynth2::setEnd2FromStart(float val)
{
	if(val > 1.0f)
		val = 1.0f;
	fb2_End = pd->presets[curProgram].b2_End.val = val;

	if(NoteMaster)
		NoteMaster->SetParameter(kb2_End, fb2_End);
}

//----------------------------------------------------------------------------
void BufferSynth2::setParameter(long index, float value)
{
	float temp45, speedval;
	//BufferSynth2Prog * ap = &programs[curProgram];

	/*if(NoteMaster)
		NoteMaster->SetParameter(index, value);*/

	switch(index)
	{
			//buffer 1
		case kb1_Start :
			if(value > fb1_End)	//use value because the new fb1_Start hasn't been calculated yet
				value = fb1_End;
			if(fb1_RetainSize < 0.5f)
			{
				fb1_Start = pd->presets[curProgram].b1_Start.val = value;
				setSize1FromStartEnd((fb1_End - fb1_Start));
			}
			else
			{
				if((value+fb1_Size) < 1.0f)
				{
					fb1_Start = pd->presets[curProgram].b1_Start.val = value;
					setEnd1FromStart((fb1_Start+fb1_Size));
				}
				else
				{
					fb1_Start = pd->presets[curProgram].b1_Start.val = (fb1_End - fb1_Size);
					setEnd1FromStart(1.0f);
				}
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Start, fb1_Start);
			break;
		case kb1_End :
			if(value < fb1_Start)
				value = fb1_Start;
			if(fb1_RetainSize < 0.5f)
			{
				fb1_End = pd->presets[curProgram].b1_End.val = value;
				setSize1FromStartEnd((fb1_End - fb1_Start));
			}
			else
			{
				if((value-fb1_Size) >= 0.0f)
				{
					fb1_End = pd->presets[curProgram].b1_End.val = value;
					setStart1FromEnd((fb1_End-fb1_Size));
				}
				else
				{
					fb1_End = pd->presets[curProgram].b1_End.val = fb1_Size;
					setStart1FromEnd(0.0f);
				}
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_End, fb1_End);
			break;
		case kb1_Size :
			fb1_Size =					pd->presets[curProgram].b1_Size.val =					value;
			if(fb1_InvertSize < 0.5f)
			{
				if(fb1_SizeFrom < 0.5f)
				{
					if((fb1_Start+fb1_Size) > 1.0f)
					{
						//setParameter(kb1_End, 1.0f);
						setEnd1FromStart(1.0f);
						//setParameter(kb1_Start, fb1_Start-((fb1_Start+fb1_Size)-1.0f));
						setStart1FromEnd(fb1_Start-((fb1_Start+fb1_Size)-1.0f));
					}
					else
						//setParameter(kb1_End, (fb1_Start+fb1_Size));
						setEnd1FromStart((fb1_Start+fb1_Size));
				}
				else
				{
					if((fb1_End-fb1_Size) < 0.0f)
					{
						//setParameter(kb1_Start, 0.0f);
						setStart1FromEnd(0.0f);
						//setParameter(kb1_End, fb1_End+((fb1_End-fb1_Size)*(-1.0f)));
						setEnd1FromStart(fb1_End+((fb1_End-fb1_Size)*(-1.0f)));
					}
					else
						//setParameter(kb1_Start, (fb1_End-fb1_Size));
						setStart1FromEnd((fb1_End-fb1_Size));
				}
				if((fb1_SizeLessThanMaxFreezes > 0.5f)&&(fb1_Size < 1.0f))
					setParameter(kb1_Freeze, 1.0f);
				else if(fb1_SizeLessThanMaxFreezes > 0.5f)
					setParameter(kb1_Freeze, 0.0f);
			}
			else
			{
				if(fb1_SizeFrom < 0.5f)
				{
					if((fb1_Start+(1.0f-fb1_Size)) > 1.0f)
					{
						//setParameter(kb1_End, 1.0f);
						setEnd1FromStart(1.0f);
						//setParameter(kb1_Start, fb1_Start-((fb1_Start+(1.0f-fb1_Size))-1.0f));
						setStart1FromEnd(fb1_Start-((fb1_Start+(1.0f-fb1_Size))-1.0f));
					}
					else
						//setParameter(kb1_End, (fb1_Start+(1.0f-fb1_Size)));
						setEnd1FromStart((fb1_Start+(1.0f-fb1_Size)));
				}
				else
				{
					if((fb1_End-(1.0f-fb1_Size)) < 0.0f)
					{
						//setParameter(kb1_Start, 0.0f);
						setStart1FromEnd(0.0f);
						//setParameter(kb1_End, fb1_End+((fb1_End-(1.0f-fb1_Size))*(-1.0f)));
						setEnd1FromStart(fb1_End+((fb1_End-(1.0f-fb1_Size))*(-1.0f)));
					}
					else
						//setParameter(kb1_Start, (fb1_End-(1.0f-fb1_Size)));
						setStart1FromEnd((fb1_End-(1.0f-fb1_Size)));
				}
				if((fb1_SizeLessThanMaxFreezes > 0.5f)&&((1.0f-fb1_Size) < 1.0f))
					setParameter(kb1_Freeze, 1.0f);
				else if(fb1_SizeLessThanMaxFreezes > 0.5f)
					setParameter(kb1_Freeze, 0.0f);
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Size, fb1_Size);
			break;
		case kb1_SizeFrom :
			fb1_SizeFrom =				pd->presets[curProgram].b1_SizeFrom.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_SizeFrom, fb1_SizeFrom);
			break;
		case kb1_RetainSize :
			fb1_RetainSize =			pd->presets[curProgram].b1_RetainSize.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_RetainSize, fb1_RetainSize);
			break;
		case kb1_Size2Tempo :
			fb1_Size2Tempo =			pd->presets[curProgram].b1_Size2Tempo.val =			value;

			if(fb1_Speed_Pitch == 0.5f)
				speedval = 1.0f;
			else if(fb1_Speed_Pitch < 0.5f)
			{
				speedval = fb1_Speed_Pitch * 1.5f;	//val = 0->0.75
				speedval += 0.25f;  //val = 0->1 (effectively 0.25->1)
			}
			else if(fb1_Speed_Pitch <= 1.0f)
			{
				speedval = fb1_Speed_Pitch - 0.5f;   //val = 0->0.5
				speedval *= 6.0f;	//val = 0->3
				speedval += 1.0f;   //val = 1->4
			}
			else
				speedval = 1.0f;

			if(fb1_Size2Tempo < (1.0f/13.0f))
				break;
			else if(fb1_Size2Tempo < (2.0f/13.0f))
			{
				temp45 = (60.0f/tempo);	//i.e. at 60bpm (44100Hz), 1 beat = 44100 samples (the entire buffer)
				temp45 /= speedval;
				temp45 *= (1.0f/16.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (3.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/8.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (4.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/6.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (5.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/4.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (6.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/3.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (7.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/2.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (8.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				//temp45 *= (1.0f/2.0f);
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (9.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 2.0f;
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (10.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 3.0f;
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (11.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 4.0f;
				setParameter(kb1_Size, temp45);
			}
			else if(fb1_Size2Tempo < (12.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 6.0f;
				setParameter(kb1_Size, temp45);
			}
			else
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 8.0f;
				setParameter(kb1_Size, temp45);
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Size2Tempo, fb1_Size2Tempo);
			break;
		case kb1_RecThreshold :
			fb1_RecThreshold =			pd->presets[curProgram].b1_RecThreshold.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_RecThreshold, fb1_RecThreshold);
			break;
		case kb1_Speed_Pitch :
			fb1_Speed_Pitch =			pd->presets[curProgram].b1_Speed_Pitch.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Speed_Pitch, fb1_Speed_Pitch);
			break;
		case kb1_Level :
			fb1_Level =					pd->presets[curProgram].b1_Level.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Level, fb1_Level);
			break;
		case kb1_Input :
			fb1_Input =					pd->presets[curProgram].b1_Input.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Input, fb1_Input);
			break;
		case kb1_StretchFile :
			fb1_StretchFile =			pd->presets[curProgram].b1_StretchFile.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_StretchFile, fb1_StretchFile);
			break;
		case kb1_LinearInterp :
			fb1_LinearInterp =			pd->presets[curProgram].b1_LinearInterp.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_LinearInterp, fb1_LinearInterp);
			break;
		case kb1_Reverse :
			fb1_Reverse =				pd->presets[curProgram].b1_Reverse.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Reverse, fb1_Reverse);
			break;
		case kb1_OnlyOPWhenFrozen :
			fb1_OnlyOPWhenFrozen =		pd->presets[curProgram].b1_OnlyOPWhenFrozen.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_OnlyOPWhenFrozen, fb1_OnlyOPWhenFrozen);
			break;
		case kb1_MIDINotesSetFreeze :
			if((fb1_MIDINotesSetFreeze > 0.5f)&&(value < 0.5f)) //so that freeze is switched off when 'MIDI Notes Set Freeze' is switched off
			{
				fb1_MIDINotesSetFreeze =	pd->presets[curProgram].b1_MIDINotesSetFreeze.val =	value;
				setParameter(kb1_Freeze, 0.0f);
			}
			else
				fb1_MIDINotesSetFreeze =	pd->presets[curProgram].b1_MIDINotesSetFreeze.val =	value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_MIDINotesSetFreeze, fb1_MIDINotesSetFreeze);
			break;
		case kb1_Freeze :
			fb1_Freeze =				pd->presets[curProgram].b1_Freeze.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Freeze, fb1_Freeze);
			break;
		case kb1_SizeLessThanMaxFreezes :
			fb1_SizeLessThanMaxFreezes =pd->presets[curProgram].b1_SizeLessThanMaxFreezes.val =value;
			/*if(value > 0.5f)
				setParameter(kb1_Size, fb1_Size);
			else
				setParameter(kb1_Freeze, 0.0f);*/

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_SizeLessThanMaxFreezes, fb1_SizeLessThanMaxFreezes);
			break;
		case kb1_InvertSize :
			fb1_InvertSize =			pd->presets[curProgram].b1_InvertSize.val =			value;
			setParameter(kb1_Size, fb1_Size);

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_InvertSize, fb1_InvertSize);
			break;
		case kb1_ReadPosition :
			fb1_ReadPosition =			pd->presets[curProgram].b1_ReadPosition.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_ReadPosition, fb1_ReadPosition);
			break;
		case kb1_ResetRPOnMIDINote :
			fb1_ResetRPOnMIDINote =			pd->presets[curProgram].b1_ResetRPOnMIDINote.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_ResetRPOnMIDINote, fb1_ResetRPOnMIDINote);
			break;
		case kb1_Pan :
			fb1_Pan =			pd->presets[curProgram].b1_Pan.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_Pan, fb1_Pan);
			break;
		case kb1_IPGain :
			fb1_IPGain =			pd->presets[curProgram].b1_IPGain.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb1_IPGain, fb1_IPGain);
			break;

			//buffer 2
		case kb2_Start :
			if(value > fb2_End)	//use value because the new fb2_Start hasn't been calculated yet
				value = fb2_End;
			if(fb2_RetainSize < 0.5f)
			{
				fb2_Start = pd->presets[curProgram].b2_Start.val = value;
				setSize2FromStartEnd((fb2_End - fb2_Start));
			}
			else
			{
				if((value+fb2_Size) < 1.0f)
				{
					fb2_Start = pd->presets[curProgram].b2_Start.val = value;
					setEnd2FromStart((fb2_Start+fb2_Size));
				}
				else
				{
					fb2_Start = pd->presets[curProgram].b2_Start.val = (fb2_End - fb2_Size);
					setEnd2FromStart(1.0f);
				}
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Start, fb2_Start);
			break;
		case kb2_End :
			if(value < fb2_Start)
				value = fb2_Start;
			if(fb2_RetainSize < 0.5f)
			{
				fb2_End = pd->presets[curProgram].b2_End.val = value;
				setSize2FromStartEnd((fb2_End - fb2_Start));
			}
			else
			{
				if((value-fb2_Size) >= 0.0f)
				{
					fb2_End = pd->presets[curProgram].b2_End.val = value;
					setStart2FromEnd((fb2_End-fb2_Size));
				}
				else
				{
					fb2_End = pd->presets[curProgram].b2_End.val = fb2_Size;
					setStart2FromEnd(0.0f);
				}
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_End, fb2_End);
			break;
		case kb2_Size :
			fb2_Size = pd->presets[curProgram].b2_Size.val = value;
			if(fb2_InvertSize < 0.5f)
			{
				if(fb2_SizeFrom < 0.5f)
				{
					if((fb2_Start+fb2_Size) > 1.0f)
					{
						//setParameter(kb2_End, 1.0f);
						setEnd2FromStart(1.0f);
						//setParameter(kb2_Start, fb2_Start-((fb2_Start+fb2_Size)-1.0f));
						setStart2FromEnd(fb2_Start-((fb2_Start+fb2_Size)-1.0f));
					}
					else
						//setParameter(kb2_End, (fb2_Start+fb2_Size));
						setEnd2FromStart((fb2_Start+fb2_Size));
				}
				else
				{
					if((fb2_End-fb2_Size) < 0.0f)
					{
						//setParameter(kb2_Start, 0.0f);
						setStart2FromEnd(0.0f);
						//setParameter(kb2_End, fb2_End+((fb2_End-fb2_Size)*(-1.0f)));
						setEnd2FromStart(fb2_End+((fb2_End-fb2_Size)*(-1.0f)));
					}
					else
						//setParameter(kb2_Start, (fb2_End-fb2_Size));
						setStart2FromEnd((fb2_End-fb2_Size));
				}
				if((fb2_SizeLessThanMaxFreezes > 0.5f)&&(fb2_Size < 1.0f))
					setParameter(kb2_Freeze, 1.0f);
				else if(fb2_SizeLessThanMaxFreezes > 0.5f)
					setParameter(kb2_Freeze, 0.0f);
			}
			else
			{
				if(fb2_SizeFrom < 0.5f)
				{
					if((fb2_Start+(1.0f-fb2_Size)) > 1.0f)
					{
						//setParameter(kb2_End, 1.0f);
						setEnd2FromStart(1.0f);
						//setParameter(kb2_Start, fb2_Start-((fb2_Start+(1.0f-fb2_Size))-1.0f));
						setStart2FromEnd(fb2_Start-((fb2_Start+(1.0f-fb2_Size))-1.0f));
					}
					else
						//setParameter(kb2_End, (fb2_Start+(1.0f-fb2_Size)));
						setEnd2FromStart((fb2_Start+(1.0f-fb2_Size)));
				}
				else
				{
					if((fb2_End-(1.0f-fb2_Size)) < 0.0f)
					{
						//setParameter(kb2_Start, 0.0f);
						setStart2FromEnd(0.0f);
						//setParameter(kb2_End, fb2_End+((fb2_End-(1.0f-fb2_Size))*(-1.0f)));
						setEnd2FromStart(fb2_End+((fb2_End-(1.0f-fb2_Size))*(-1.0f)));
					}
					else
						//setParameter(kb2_Start, (fb2_End-(1.0f-fb2_Size)));
						setStart2FromEnd((fb2_End-(1.0f-fb2_Size)));
				}
				if((fb2_SizeLessThanMaxFreezes > 0.5f)&&((1.0f-fb2_Size) < 1.0f))
					setParameter(kb2_Freeze, 1.0f);
				else if(fb2_SizeLessThanMaxFreezes > 0.5f)
					setParameter(kb2_Freeze, 0.0f);
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Size, fb2_Size);
			break;
		case kb2_SizeFrom :
			fb2_SizeFrom =				pd->presets[curProgram].b2_SizeFrom.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_SizeFrom, fb2_SizeFrom);
			break;
		case kb2_RetainSize :
			fb2_RetainSize =			pd->presets[curProgram].b2_RetainSize.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_RetainSize, fb2_RetainSize);
			break;
		case kb2_Size2Tempo :
			fb2_Size2Tempo =			pd->presets[curProgram].b2_Size2Tempo.val =			value;

			if(fb2_Speed_Pitch == 0.5f)
				speedval = 1.0f;
			else if(fb2_Speed_Pitch < 0.5f)
			{
				speedval = fb2_Speed_Pitch * 1.5f;	//val = 0->0.75
				speedval += 0.25f;//val = 0->1 (effectively 0.25->1)
			}
			else if(fb2_Speed_Pitch <= 1.0f)
			{
				speedval = fb2_Speed_Pitch - 0.5f;
				speedval *= 6.0f;
				speedval += 1.0f;
			}
			else
				speedval = 1.0f;

			if(fb2_Size2Tempo < (1.0f/13.0f))
				break;
			else if(fb2_Size2Tempo < (2.0f/13.0f))
			{
				temp45 = (60.0f/tempo);	//i.e. at 60bpm (44100Hz), 1 beat = 44100 samples (the entire buffer)
				temp45 /= speedval;
				temp45 *= (1.0f/16.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (3.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/8.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (4.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/6.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (5.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/4.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (6.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/3.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (7.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= (1.0f/2.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (8.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				//temp45 *= (1.0f/2.0f);
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (9.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 2.0f;
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (10.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 3.0f;
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (11.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 4.0f;
				setParameter(kb2_Size, temp45);
			}
			else if(fb2_Size2Tempo < (12.0f/13.0f))
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 6.0f;
				setParameter(kb2_Size, temp45);
			}
			else
			{
				temp45 = (60.0f/tempo);
				temp45 /= speedval;
				temp45 *= 8.0f;
				setParameter(kb2_Size, temp45);
			}

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Size2Tempo, fb2_Size2Tempo);
			break;
		case kb2_RecThreshold :
			fb2_RecThreshold =			pd->presets[curProgram].b2_RecThreshold.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_RecThreshold, fb2_RecThreshold);
			break;
		case kb2_Speed_Pitch :
			fb2_Speed_Pitch =			pd->presets[curProgram].b2_Speed_Pitch.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Speed_Pitch, fb2_Speed_Pitch);
			break;
		case kb2_Level :
			fb2_Level =					pd->presets[curProgram].b2_Level.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Level, fb2_Level);
			break;
		case kb2_Input :
			fb2_Input =					pd->presets[curProgram].b2_Input.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Input, fb2_Input);
			break;
		case kb2_StretchFile :
			fb2_StretchFile =			pd->presets[curProgram].b2_StretchFile.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_StretchFile, fb2_StretchFile);
			break;
		case kb2_LinearInterp :
			fb2_LinearInterp =			pd->presets[curProgram].b2_LinearInterp.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_LinearInterp, fb2_LinearInterp);
			break;
		case kb2_Reverse :
			fb2_Reverse =				pd->presets[curProgram].b2_Reverse.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Reverse, fb2_Reverse);
			break;
		case kb2_OnlyOPWhenFrozen :
			fb2_OnlyOPWhenFrozen =		pd->presets[curProgram].b2_OnlyOPWhenFrozen.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_OnlyOPWhenFrozen, fb2_OnlyOPWhenFrozen);
			break;
		case kb2_MIDINotesSetFreeze :
			if((fb2_MIDINotesSetFreeze > 0.5f)&&(value < 0.5f)) //so that freeze is switched off when 'MIDI Notes Set Freeze' is switched off
			{
				fb2_MIDINotesSetFreeze =	pd->presets[curProgram].b2_MIDINotesSetFreeze.val =	value;
				setParameter(kb2_Freeze, 0.0f);
			}
			else
				fb2_MIDINotesSetFreeze =	pd->presets[curProgram].b2_MIDINotesSetFreeze.val =	value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_MIDINotesSetFreeze, fb2_MIDINotesSetFreeze);
			break;
		case kb2_Freeze :
			fb2_Freeze =				pd->presets[curProgram].b2_Freeze.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Freeze, fb2_Freeze);
			break;
		case kb2_ModDestination :
			fb2_ModDestination =		pd->presets[curProgram].b2_ModDestination.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_ModDestination, fb2_ModDestination);
			break;
		case kb2_ModDepth :
			fb2_ModDepth =				pd->presets[curProgram].b2_ModDepth.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_ModDepth, fb2_ModDepth);
			break;
		case kb2_SizeLessThanMaxFreezes :
			fb2_SizeLessThanMaxFreezes =pd->presets[curProgram].b2_SizeLessThanMaxFreezes.val =value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_SizeLessThanMaxFreezes, fb2_SizeLessThanMaxFreezes);
			break;
		case kb2_InvertSize :
			fb2_InvertSize =			pd->presets[curProgram].b2_InvertSize.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_InvertSize, fb2_InvertSize);
			break;
		case kb2_Envelope :
			fb2_Envelope =				pd->presets[curProgram].b2_Envelope.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Envelope, fb2_Envelope);
			break;
		case kb2_ReadPosition :
			fb2_ReadPosition =			pd->presets[curProgram].b2_ReadPosition.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_ReadPosition, fb2_ReadPosition);
			break;
		case kb2_ResetRPOnMIDINote :
			fb2_ResetRPOnMIDINote =			pd->presets[curProgram].b2_ResetRPOnMIDINote.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_ResetRPOnMIDINote, fb2_ResetRPOnMIDINote);
			break;
		case kb2_Pan :
			fb2_Pan =			pd->presets[curProgram].b2_Pan.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_Pan, fb2_Pan);
			break;
		case kb2_IPGain :
			fb2_IPGain =			pd->presets[curProgram].b2_IPGain.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kb2_IPGain, fb2_IPGain);
			break;

			//amplitude envelope
		case kae_OnOff :
			fae_OnOff =					pd->presets[curProgram].ae_OnOff.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_OnOff, fae_OnOff);
			break;
		case kae_Attack :
			fae_Attack =				pd->presets[curProgram].ae_Attack.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_Attack, fae_Attack);
			break;
		case kae_Decay :
			fae_Decay =					pd->presets[curProgram].ae_Decay.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_Decay, fae_Decay);
			break;
		case kae_Sustain :
			fae_Sustain =				pd->presets[curProgram].ae_Sustain.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_Sustain, fae_Sustain);
			break;
		case kae_Release :
			fae_Release =				pd->presets[curProgram].ae_Release.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_Release, fae_Release);
			break;
		case kae_SegmentTime :
			fae_SegmentTime =			pd->presets[curProgram].ae_SegmentTime.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_SegmentTime, fae_SegmentTime);
			break;
		/*case kae_FreezeTriggers :
			fae_FreezeTriggers =		pd->presets[curProgram].ae_FreezeTriggers.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_FreezeTriggers, fae_FreezeTriggers);
			break;
		case kae_MIDINotesTrigger :
			fae_MIDINotesTrigger =		pd->presets[curProgram].ae_MIDINotesTrigger.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(kae_MIDINotesTrigger, fae_MIDINotesTrigger);
			break;*/

			//second envelope
		case ke2_Attack :
			fe2_Attack =				pd->presets[curProgram].e2_Attack.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_Attack, fe2_Attack);
			break;
			break;
		case ke2_Decay :
			fe2_Decay =					pd->presets[curProgram].e2_Decay.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_Decay, fe2_Decay);
			break;
		case ke2_Sustain :
			fe2_Sustain =				pd->presets[curProgram].e2_Sustain.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_Sustain, fe2_Sustain);
			break;
		case ke2_Release :
			fe2_Release =				pd->presets[curProgram].e2_Release.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_Release, fe2_Release);
			break;
		case ke2_SegmentTime :
			fe2_SegmentTime =			pd->presets[curProgram].e2_SegmentTime.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_SegmentTime, fe2_SegmentTime);
			break;
		case ke2_MIDINotesTrigger :
			fe2_MIDINotesTrigger =		pd->presets[curProgram].e2_MIDINotesTrigger.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_MIDINotesTrigger, fe2_MIDINotesTrigger);
			break;
		case ke2_BarStartTriggers :
			fe2_BarStartTriggers =		pd->presets[curProgram].e2_BarStartTriggers.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_BarStartTriggers, fe2_BarStartTriggers);
			break;
		case ke2_Destination :
			fe2_Destination =			pd->presets[curProgram].e2_Destination.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_Destination, fe2_Destination);
			break;
		case ke2_Direction :
			fe2_Direction =				pd->presets[curProgram].e2_Direction.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_Direction, fe2_Direction);
			break;
		case ke2_ModDepth :
			fe2_ModDepth =				pd->presets[curProgram].e2_ModDepth.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(ke2_ModDepth, fe2_ModDepth);
			break;

			//LFO1
		case klfo1_Freq_Note :
			flfo1_Freq_Note =			pd->presets[curProgram].lfo1_Freq_Note.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_Freq_Note, flfo1_Freq_Note);
			break;
		case klfo1_TempoSync :
			flfo1_TempoSync =			pd->presets[curProgram].lfo1_TempoSync.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_TempoSync, flfo1_TempoSync);
			break;
		case klfo1_Waveform :
			flfo1_Waveform =			pd->presets[curProgram].lfo1_Waveform.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_Waveform, flfo1_Waveform);
			break;
		case klfo1_BarStartResets :
			flfo1_BarStartResets =		pd->presets[curProgram].lfo1_BarStartResets.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_BarStartResets, flfo1_BarStartResets);
			break;
		case klfo1_MIDINotesReset :
			flfo1_MIDINotesReset =		pd->presets[curProgram].lfo1_MIDINotesReset.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_MIDINotesReset, flfo1_MIDINotesReset);
			break;
		case klfo1_Destination :
			flfo1_Destination =			pd->presets[curProgram].lfo1_Destination.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_Destination, flfo1_Destination);
			break;
		case klfo1_ModDepth :
			flfo1_ModDepth =			pd->presets[curProgram].lfo1_ModDepth.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo1_ModDepth, flfo1_ModDepth);
			break;

			//LFO2
		case klfo2_Freq_Note :
			flfo2_Freq_Note =			pd->presets[curProgram].lfo2_Freq_Note.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_Freq_Note, flfo2_Freq_Note);
			break;
		case klfo2_TempoSync :
			flfo2_TempoSync =			pd->presets[curProgram].lfo2_TempoSync.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_TempoSync, flfo2_TempoSync);
			break;
		case klfo2_Waveform :
			flfo2_Waveform =			pd->presets[curProgram].lfo2_Waveform.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_Waveform, flfo2_Waveform);
			break;
		case klfo2_BarStartResets :
			flfo2_BarStartResets =		pd->presets[curProgram].lfo2_BarStartResets.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_BarStartResets, flfo2_BarStartResets);
			break;
		case klfo2_MIDINotesReset :
			flfo2_MIDINotesReset =		pd->presets[curProgram].lfo2_MIDINotesReset.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_MIDINotesReset, flfo2_MIDINotesReset);
			break;
		case klfo2_Destination :
			flfo2_Destination =			pd->presets[curProgram].lfo2_Destination.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_Destination, flfo2_Destination);
			break;
		case klfo2_ModDepth :
			flfo2_ModDepth =			pd->presets[curProgram].lfo2_ModDepth.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(klfo2_ModDepth, flfo2_ModDepth);
			break;

			//filter
		case kfilt_OnOff :
			ffilt_OnOff =				pd->presets[curProgram].filt_OnOff.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kfilt_OnOff, ffilt_OnOff);
			break;
		case kfilt_Cutoff :
			ffilt_Cutoff =				pd->presets[curProgram].filt_Cutoff.val =				value;

			/*if(NoteMaster)
				NoteMaster->SetParameter(kfilt_Cutoff, ffilt_Cutoff);*/
			break;
		case kfilt_Resonance :
			ffilt_Resonance =			pd->presets[curProgram].filt_Resonance.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(kfilt_Resonance, ffilt_Resonance);
			break;
		case kfilt_Type :
			ffilt_Type =				pd->presets[curProgram].filt_Type.val =				value;

			if(NoteMaster)
				NoteMaster->SetParameter(kfilt_Type, ffilt_Type);
			break;

			//settings
		case ksett_MIDILearn :
			fsett_MIDILearn =			pd->presets[curProgram].sett_MIDILearn.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(ksett_MIDILearn, fsett_MIDILearn);
			break;
		case ksett_SynthMode :
			fsett_SynthMode =			pd->presets[curProgram].sett_SynthMode.val =			value;

			if(NoteMaster)
				NoteMaster->SetParameter(ksett_SynthMode, fsett_SynthMode);
			break;
		case ksett_PolyphonicMode :
			fsett_PolyphonicMode =		pd->presets[curProgram].sett_PolyphonicMode.val =		value;

			if(NoteMaster)
				NoteMaster->SetParameter(ksett_PolyphonicMode, fsett_PolyphonicMode);
			break;
		case ksett_PitchCorrection :
			fsett_PitchCorrection =	pd->presets[curProgram].sett_PitchCorrection.val =	value;

			if(NoteMaster)
				NoteMaster->SetParameter(ksett_PitchCorrection, fsett_PitchCorrection);
			break;
		case ksett_SaveBufferContents :
			fsett_SaveBufferContents =	pd->presets[curProgram].sett_SaveBufferContents.val =	value;

			if(NoteMaster)
				NoteMaster->SetParameter(ksett_SaveBufferContents, fsett_SaveBufferContents);
			break;

			//output
		case kop_Mix :
			fop_Mix =					pd->presets[curProgram].op_Mix.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kop_Mix, fop_Mix);
			break;
		case kop_Level :
			fop_Level =					pd->presets[curProgram].op_Level.val =					value;

			if(NoteMaster)
				NoteMaster->SetParameter(kop_Level, fop_Level);
			break;
	}
}

//----------------------------------------------------------------------------
float BufferSynth2::getParameter(long index)
{
	float v = 0;

	switch (index)
	{
		//buffer 1
		case kb1_Start :				v = fb1_Start; break;
		case kb1_End :					v = fb1_End; break;
		case kb1_Size :					v = fb1_Size; break;
		case kb1_SizeFrom :				v = fb1_SizeFrom; break;
		case kb1_RetainSize :			v = fb1_RetainSize; break;
		case kb1_Size2Tempo :			v = fb1_Size2Tempo; break;
		case kb1_RecThreshold :			v = fb1_RecThreshold; break;
		case kb1_Speed_Pitch :			v = fb1_Speed_Pitch; break;
		case kb1_Level :				v = fb1_Level; break;
		case kb1_Input :				v = fb1_Input; break;
		case kb1_StretchFile :			v = fb1_StretchFile; break;
		case kb1_LinearInterp :			v = fb1_LinearInterp; break;
		case kb1_Reverse :				v = fb1_Reverse; break;
		case kb1_OnlyOPWhenFrozen :		v = fb1_OnlyOPWhenFrozen; break;
		case kb1_MIDINotesSetFreeze :	v = fb1_MIDINotesSetFreeze; break;
		case kb1_Freeze :				v = fb1_Freeze; break;
		case kb1_SizeLessThanMaxFreezes:v = fb1_SizeLessThanMaxFreezes; break;
		case kb1_InvertSize :			v = fb1_InvertSize; break;
		case kb1_ReadPosition :			v = fb1_ReadPosition; break;
		case kb1_ResetRPOnMIDINote :	v = fb1_ResetRPOnMIDINote; break;
		case kb1_Pan :					v = fb1_Pan; break;
		case kb1_IPGain :				v = fb1_IPGain; break;

		//buffer 2
		case kb2_Start :				v = fb2_Start; break;
		case kb2_End :					v = fb2_End; break;
		case kb2_Size :					v = fb2_Size; break;
		case kb2_SizeFrom :				v = fb2_SizeFrom; break;
		case kb2_RetainSize :			v = fb2_RetainSize; break;
		case kb2_Size2Tempo :			v = fb2_Size2Tempo; break;
		case kb2_RecThreshold :			v = fb2_RecThreshold; break;
		case kb2_Speed_Pitch :			v = fb2_Speed_Pitch; break;
		case kb2_Level :				v = fb2_Level; break;
		case kb2_Input :				v = fb2_Input; break;
		case kb2_StretchFile :			v = fb2_StretchFile; break;
		case kb2_LinearInterp :			v = fb2_LinearInterp; break;
		case kb2_Reverse :				v = fb2_Reverse; break;
		case kb2_OnlyOPWhenFrozen :		v = fb2_OnlyOPWhenFrozen; break;
		case kb2_MIDINotesSetFreeze :	v = fb2_MIDINotesSetFreeze; break;
		case kb2_Freeze :				v = fb2_Freeze; break;
		case kb2_ModDestination :		v = fb2_ModDestination; break;
		case kb2_ModDepth :				v = fb2_ModDepth; break;
		case kb2_SizeLessThanMaxFreezes:v = fb2_SizeLessThanMaxFreezes; break;
		case kb2_InvertSize :			v = fb2_InvertSize; break;
		case kb2_Envelope :				v = fb2_Envelope; break;
		case kb2_ReadPosition :			v = fb2_ReadPosition; break;
		case kb2_ResetRPOnMIDINote :	v = fb2_ResetRPOnMIDINote; break;
		case kb2_Pan :					v = fb2_Pan; break;
		case kb2_IPGain :				v = fb2_IPGain; break;

		//amplitude envelope
		case kae_OnOff :				v = fae_OnOff; break;
		case kae_Attack :				v = fae_Attack; break;
		case kae_Decay :				v = fae_Decay; break;
		case kae_Sustain :				v = fae_Sustain; break;
		case kae_Release :				v = fae_Release; break;
		case kae_SegmentTime :			v = fae_SegmentTime; break;
		/*case kae_FreezeTriggers :		v = fae_FreezeTriggers; break;
		case kae_MIDINotesTrigger :		v = fae_MIDINotesTrigger; break;*/

		//second envelope
		case ke2_Attack :				v = fe2_Attack; break;
		case ke2_Decay :				v = fe2_Decay; break;
		case ke2_Sustain :				v = fe2_Sustain; break;
		case ke2_Release :				v = fe2_Release; break;
		case ke2_SegmentTime :			v = fe2_SegmentTime; break;
		case ke2_MIDINotesTrigger :		v = fe2_MIDINotesTrigger; break;
		case ke2_BarStartTriggers :		v = fe2_BarStartTriggers; break;
		case ke2_Destination :			v = fe2_Destination; break;
		case ke2_Direction :			v = fe2_Direction; break;
		case ke2_ModDepth :				v = fe2_ModDepth; break;

		//LFO1
		case klfo1_Freq_Note :			v = flfo1_Freq_Note; break;
		case klfo1_TempoSync :			v = flfo1_TempoSync; break;
		case klfo1_Waveform :			v = flfo1_Waveform; break;
		case klfo1_BarStartResets :		v = flfo1_BarStartResets; break;
		case klfo1_MIDINotesReset :		v = flfo1_MIDINotesReset; break;
		case klfo1_Destination :		v = flfo1_Destination; break;
		case klfo1_ModDepth :			v = flfo1_ModDepth; break;

		//LFO2
		case klfo2_Freq_Note :			v = flfo2_Freq_Note; break;
		case klfo2_TempoSync :			v = flfo2_TempoSync; break;
		case klfo2_Waveform :			v = flfo2_Waveform; break;
		case klfo2_BarStartResets :		v = flfo2_BarStartResets; break;
		case klfo2_MIDINotesReset :		v = flfo2_MIDINotesReset; break;
		case klfo2_Destination :		v = flfo2_Destination; break;
		case klfo2_ModDepth :			v = flfo2_ModDepth; break;

		//filter
		case kfilt_OnOff :				v = ffilt_OnOff; break;
		case kfilt_Cutoff :				v = ffilt_Cutoff; break;
		case kfilt_Resonance :			v = ffilt_Resonance; break;
		case kfilt_Type :				v = ffilt_Type; break;

		//settings
		case ksett_MIDILearn :			v = fsett_MIDILearn; break;
		case ksett_SynthMode :			v = fsett_SynthMode; break;
		case ksett_PolyphonicMode :		v = fsett_PolyphonicMode; break;
		case ksett_PitchCorrection :	v = fsett_PitchCorrection; break;
		case ksett_SaveBufferContents : v = fsett_SaveBufferContents; break;

		//output
		case kop_Mix :					v = fop_Mix; break;
		case kop_Level :				v = fop_Level; break;
	}
	return v;
}

//----------------------------------------------------------------------------
void BufferSynth2::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		//buffer 1
		case kb1_Start :				strcpy(label, "samples"); break;
		case kb1_End :					strcpy(label, "samples"); break;
		case kb1_Size :					strcpy(label, "seconds"); break;
		case kb1_SizeFrom :				strcpy(label, " "); break;
		case kb1_RetainSize :			strcpy(label, " "); break;
		case kb1_Size2Tempo :			strcpy(label, " "); break;
		case kb1_RecThreshold :			strcpy(label, "dB"); break;
		case kb1_Speed_Pitch :
			if(fsett_SynthMode < 0.5f)
				strcpy(label, "x");
			else
				strcpy(label, "oct");
			break;
		case kb1_Level :				strcpy(label, "dB"); break;
		case kb1_Input :				strcpy(label, " "); break;
		case kb1_StretchFile :			strcpy(label, " "); break;
		case kb1_LinearInterp :			strcpy(label, " "); break;
		case kb1_Reverse :				strcpy(label, " "); break;
		case kb1_OnlyOPWhenFrozen :		strcpy(label, " "); break;
		case kb1_MIDINotesSetFreeze :	strcpy(label, " "); break;
		case kb1_Freeze :				strcpy(label, " "); break;
		case kb1_SizeLessThanMaxFreezes:strcpy(label, " "); break;
		case kb1_InvertSize :			strcpy(label, " "); break;
		case kb1_ReadPosition :			strcpy(label, "samples"); break;
		case kb1_ResetRPOnMIDINote :	strcpy(label, " "); break;
		case kb1_Pan :
			if(fb1_Pan < 0.5f)
				strcpy(label, "l");
			else if(fb1_Pan > 0.5f)
				strcpy(label, "r");
			else
				strcpy(label, " ");
			break;
		case kb1_IPGain :				strcpy(label, "dB"); break;

		//buffer 2
		case kb2_Start :				strcpy(label, "samples"); break;
		case kb2_End :					strcpy(label, "samples"); break;
		case kb2_Size :					strcpy(label, "seconds"); break;
		case kb2_SizeFrom :				strcpy(label, " "); break;
		case kb2_RetainSize :			strcpy(label, " "); break;
		case kb2_Size2Tempo :			strcpy(label, " "); break;
		case kb2_RecThreshold :			strcpy(label, "dB"); break;
		case kb2_Speed_Pitch :
			if(fsett_SynthMode < 0.5f)
				strcpy(label, "x");
			else
				strcpy(label, "oct");
			break;
		case kb2_Level :				strcpy(label, "dB"); break;
		case kb2_Input :				strcpy(label, " "); break;
		case kb2_StretchFile :			strcpy(label, " "); break;
		case kb2_LinearInterp :			strcpy(label, " "); break;
		case kb2_Reverse :				strcpy(label, " "); break;
		case kb2_OnlyOPWhenFrozen :		strcpy(label, " "); break;
		case kb2_MIDINotesSetFreeze :	strcpy(label, " "); break;
		case kb2_Freeze :				strcpy(label, " "); break;
		case kb2_ModDestination :		strcpy(label, " "); break;
		case kb2_ModDepth :				strcpy(label, "dB"); break;
		case kb2_SizeLessThanMaxFreezes:strcpy(label, " "); break;
		case kb2_InvertSize :			strcpy(label, " "); break;
		case kb2_Envelope :				strcpy(label, " "); break;
		case kb2_ReadPosition :			strcpy(label, "samples"); break;
		case kb2_ResetRPOnMIDINote :	strcpy(label, " "); break;
		case kb2_Pan :
			if(fb2_Pan < 0.5f)
				strcpy(label, "l");
			else if(fb2_Pan > 0.5f)
				strcpy(label, "r");
			else
				strcpy(label, " ");
			break;
		case kb2_IPGain :				strcpy(label, "dB"); break;

		//amplitude envelope
		case kae_OnOff :				strcpy(label, " "); break;
		case kae_Attack :				strcpy(label, "seconds"); break;
		case kae_Decay :				strcpy(label, "seconds"); break;
		case kae_Sustain :				strcpy(label, "dB"); break;
		case kae_Release :				strcpy(label, "seconds"); break;
		case kae_SegmentTime :			strcpy(label, "seconds"); break;
		/*case kae_FreezeTriggers :		strcpy(label, " "); break;
		case kae_MIDINotesTrigger :		strcpy(label, " "); break;*/

		//second envelope
		case ke2_Attack :				strcpy(label, "seconds"); break;
		case ke2_Decay :				strcpy(label, "seconds"); break;
		case ke2_Sustain :				strcpy(label, "dB"); break;
		case ke2_Release :				strcpy(label, "seconds"); break;
		case ke2_SegmentTime :			strcpy(label, "seconds"); break;
		case ke2_MIDINotesTrigger :		strcpy(label, " "); break;
		case ke2_BarStartTriggers :		strcpy(label, " "); break;
		case ke2_Destination :			strcpy(label, " "); break;
		case ke2_Direction :			strcpy(label, " "); break;
		case ke2_ModDepth :				strcpy(label, "dB"); break;

		//LFO 1
		case klfo1_Freq_Note :
			if(flfo1_TempoSync < 0.5f)
				strcpy(label, "Hz");
			else
				strcpy(label, "Beat");
			break;
		case klfo1_TempoSync :			strcpy(label, " "); break;
		case klfo1_Waveform :			strcpy(label, " "); break;
		case klfo1_BarStartResets :		strcpy(label, " "); break;
		case klfo1_MIDINotesReset :		strcpy(label, " "); break;
		case klfo1_Destination :		strcpy(label, " "); break;
		case klfo1_ModDepth :			strcpy(label, "dB"); break;

		//LFO 2
		case klfo2_Freq_Note :
			if(flfo2_TempoSync < 0.5f)
				strcpy(label, "Hz");
			else
				strcpy(label, "Beat");
			break;
		case klfo2_TempoSync :			strcpy(label, " "); break;
		case klfo2_Waveform :			strcpy(label, " "); break;
		case klfo2_BarStartResets :		strcpy(label, " "); break;
		case klfo2_MIDINotesReset :		strcpy(label, " "); break;
		case klfo2_Destination :		strcpy(label, " "); break;
		case klfo2_ModDepth :			strcpy(label, "dB"); break;

		//filter
		case kfilt_OnOff :				strcpy(label, " "); break;
		case kfilt_Cutoff :				strcpy(label, "Hz"); break;
		case kfilt_Resonance :			strcpy(label, " "); break;
		case kfilt_Type :				strcpy(label, " "); break;

		//settings
		case ksett_MIDILearn :			strcpy(label, " "); break;
		case ksett_SynthMode :			strcpy(label, " "); break;
		case ksett_PolyphonicMode :		strcpy(label, " "); break;
		case ksett_PitchCorrection :	strcpy(label, " "); break;
		case ksett_SaveBufferContents : strcpy(label, " "); break;

		//output
		case kop_Mix :					strcpy(label, "% wet"); break;
		case kop_Level :				strcpy(label, "dB"); break;
	}
}

//----------------------------------------------------------------------------
//gets values for parameter display - see 'AudioEffect.hpp' for others (float2string,
//Hz2string etc.)
//----------------------------------------------------------------------------
void BufferSynth2::getParameterDisplay(long index, char *text)
{
	float tempfloat;

	switch (index)
	{
		//buffer 1
		case kb1_Start :				long2string((long)(BUFFERSIZE*fb1_Start), text); break;
		case kb1_End :					long2string((long)(BUFFERSIZE*fb1_End), text); break;
		case kb1_Size :					float2string(fb1_Size, text); break;
		case kb1_SizeFrom :				sizefrom2string(fb1_SizeFrom, text); break;
		case kb1_RetainSize :			onoff2string(fb1_RetainSize, text); break;
		case kb1_Size2Tempo :			size2tempo2string(fb1_Size2Tempo, text); break;
		case kb1_RecThreshold :			dB2string(fb1_RecThreshold, text); break;
		case kb1_Speed_Pitch :
			if(fsett_SynthMode < 0.5f)
			{
				if(fb1_Speed_Pitch == 0.5f)
					tempfloat = 1.0f;
				else if(fb1_Speed_Pitch < 0.5f)
				{
					tempfloat = fb1_Speed_Pitch * 1.5f;
					tempfloat += 0.25f;
				}
				else if(fb1_Speed_Pitch <= 1.0f)
				{
					tempfloat = fb1_Speed_Pitch - 0.5f;
					tempfloat *= 6.0f;
					tempfloat += 1.0f;
				}
			}
			else
			{
				tempfloat = fb1_Speed_Pitch - 0.5f;
				tempfloat *= 4.0f;
			}
			float2string(tempfloat, text);
			break;
		case kb1_Level :				dB2string((2.0f*fb1_Level), text); break;
		case kb1_Input :				input2string(fb1_Input, text); break;
		case kb1_StretchFile :			onoff2string(fb1_StretchFile, text); break;
		case kb1_LinearInterp :			onoff2string(fb1_LinearInterp, text); break;
		case kb1_Reverse :				onoff2string(fb1_Reverse, text); break;
		case kb1_OnlyOPWhenFrozen :		onoff2string(fb1_OnlyOPWhenFrozen, text); break;
		case kb1_MIDINotesSetFreeze :	onoff2string(fb1_MIDINotesSetFreeze, text); break;
		case kb1_Freeze :				onoff2string(fb1_Freeze, text); break;
		case kb1_SizeLessThanMaxFreezes:onoff2string(fb1_SizeLessThanMaxFreezes, text); break;
		case kb1_InvertSize :			onoff2string(fb1_InvertSize, text); break;
		case kb1_ReadPosition :			long2string((long)(BUFFERSIZE*fb1_ReadPosition), text); break;
		case kb1_ResetRPOnMIDINote :	onoff2string(fb1_ResetRPOnMIDINote, text); break;
		case kb1_Pan :					float2string((float)fabs((fb1_Pan-0.5f)*2.0f), text); break;
		case kb1_IPGain :				dB2string((2.0f*fb1_IPGain), text); break;

		//buffer 2
		case kb2_Start :				long2string((long)(BUFFERSIZE*fb2_Start), text); break;
		case kb2_End :					long2string((long)(BUFFERSIZE*fb2_End), text); break;
		case kb2_Size :					float2string(fb2_Size, text); break;
		case kb2_SizeFrom :				sizefrom2string(fb2_SizeFrom, text); break;
		case kb2_RetainSize :			onoff2string(fb2_RetainSize, text); break;
		case kb2_Size2Tempo :			size2tempo2string(fb2_Size2Tempo, text); break;
		case kb2_RecThreshold :			dB2string(fb2_RecThreshold, text); break;
		case kb2_Speed_Pitch :
			if(fsett_SynthMode < 0.5f)
			{
				if(fb2_Speed_Pitch == 0.5f)
					tempfloat = 1.0f;
				else if(fb2_Speed_Pitch < 0.5f)
				{
					tempfloat = fb2_Speed_Pitch * 1.5f;
					tempfloat += 0.25f;
				}
				else if(fb2_Speed_Pitch <= 1.0f)
				{
					tempfloat = fb2_Speed_Pitch - 0.5f;
					tempfloat *= 6.0f;
					tempfloat += 1.0f;
				}
			}
			else
			{
				tempfloat = fb2_Speed_Pitch - 0.5f;
				tempfloat *= 4.0f;
			}
			float2string(tempfloat, text);
			break;
		case kb2_Level :				dB2string((2.0f*fb2_Level), text); break;
		case kb2_Input :				input2string(fb2_Input, text); break;
		case kb2_StretchFile :			onoff2string(fb2_StretchFile, text); break;
		case kb2_LinearInterp :			onoff2string(fb2_LinearInterp, text); break;
		case kb2_Reverse :				onoff2string(fb2_Reverse, text); break;
		case kb2_OnlyOPWhenFrozen :		onoff2string(fb2_OnlyOPWhenFrozen, text); break;
		case kb2_MIDINotesSetFreeze :	onoff2string(fb2_MIDINotesSetFreeze, text); break;
		case kb2_Freeze :				onoff2string(fb2_Freeze, text); break;
		case kb2_ModDestination :		b2dest2string(fb2_ModDestination, text); break;
		case kb2_ModDepth :				dB2string(fb2_ModDepth, text); break;
		case kb2_SizeLessThanMaxFreezes:onoff2string(fb2_SizeLessThanMaxFreezes, text); break;
		case kb2_InvertSize :			onoff2string(fb2_InvertSize, text); break;
		case kb2_Envelope :				onoff2string(fb2_Envelope, text); break;
		case kb2_ReadPosition :			long2string((long)(BUFFERSIZE*fb2_ReadPosition), text); break;
		case kb2_ResetRPOnMIDINote :	onoff2string(fb2_ResetRPOnMIDINote, text); break;
		case kb2_Pan :					float2string((float)fabs((fb2_Pan-0.5f)*2.0f), text); break;
		case kb2_IPGain :				dB2string((2.0f*fb2_IPGain), text); break;

		//amplitude envelope
		case kae_OnOff :				onoff2string(fae_OnOff, text); break;
		case kae_Attack :				float2string((fae_Attack*(fae_SegmentTime*10.0f)), text); break;
		case kae_Decay :				float2string((fae_Decay*(fae_SegmentTime*10.0f)), text); break;
		case kae_Sustain :				dB2string(fae_Sustain, text); break;
		case kae_Release :				float2string((fae_Release*(fae_SegmentTime*10.0f)), text); break;
		case kae_SegmentTime :			float2string((fae_SegmentTime*10.0f), text); break;
		/*case kae_FreezeTriggers :		onoff2string(fae_FreezeTriggers, text); break;
		case kae_MIDINotesTrigger :		onoff2string(fae_MIDINotesTrigger, text); break;*/

		//second envelope
		case ke2_Attack :				float2string((fe2_Attack*(fe2_SegmentTime*10.0f)), text); break;
		case ke2_Decay :				float2string((fe2_Decay*(fe2_SegmentTime*10.0f)), text); break;
		case ke2_Sustain :				dB2string(fe2_Sustain, text); break;
		case ke2_Release :				float2string((fe2_Release*(fe2_SegmentTime*10.0f)), text); break;
		case ke2_SegmentTime :			float2string((fe2_SegmentTime*10.0f), text); break;
		case ke2_MIDINotesTrigger :		onoff2string(fe2_MIDINotesTrigger, text); break;
		case ke2_BarStartTriggers :		onoff2string(fe2_BarStartTriggers, text); break;
		case ke2_Destination :			e2dest2string(fe2_Destination, text); break;
		case ke2_Direction :			e2invert2string(fe2_Direction, text); break;
		case ke2_ModDepth :				dB2string(fe2_ModDepth, text); break;

		//LFO1
		case klfo1_Freq_Note :
			if(flfo1_TempoSync < 0.5f)
				float2string((30.0f*flfo1_Freq_Note), text);
			else
			{
				lfonote2string(flfo1_Freq_Note, text);
			}
			break;
		case klfo1_TempoSync :			temposync2string(flfo1_TempoSync, text); break;
		case klfo1_Waveform :			wave2string(flfo1_Waveform, text); break;
		case klfo1_BarStartResets :		onoff2string(flfo1_BarStartResets, text); break;
		case klfo1_MIDINotesReset :		onoff2string(flfo1_MIDINotesReset, text); break;
		case klfo1_Destination :		lfodest2string(flfo1_Destination, text); break;
		case klfo1_ModDepth :			dB2string(flfo1_ModDepth, text); break;

		//LFO2
		case klfo2_Freq_Note :
			if(flfo2_TempoSync < 0.5f)
				float2string((30.0f*flfo2_Freq_Note), text);
			else
			{
				lfonote2string(flfo2_Freq_Note, text);
			}
			break;
		case klfo2_TempoSync :			temposync2string(flfo2_TempoSync, text); break;
		case klfo2_Waveform :			wave2string(flfo2_Waveform, text); break;
		case klfo2_BarStartResets :		onoff2string(flfo2_BarStartResets, text); break;
		case klfo2_MIDINotesReset :		onoff2string(flfo2_MIDINotesReset, text); break;
		case klfo2_Destination :		lfodest2string(flfo2_Destination, text); break;
		case klfo2_ModDepth :			dB2string(flfo2_ModDepth, text); break;

		//filter
		case kfilt_OnOff :				onoff2string(ffilt_OnOff, text); break;
		case kfilt_Cutoff :				float2string((ffilt_Cutoff*22000.0f), text); break;
		case kfilt_Resonance :			float2string(ffilt_Resonance, text); break;
		case kfilt_Type :				filttype2string(ffilt_Type, text); break;

		//settings
		case ksett_MIDILearn :			onoff2string(fsett_MIDILearn, text); break;
		case ksett_SynthMode :			onoff2string(fsett_SynthMode, text); break;
		case ksett_PolyphonicMode :		onoff2string(fsett_PolyphonicMode, text); break;
		case ksett_PitchCorrection :	pcorrection2string(fsett_PitchCorrection, text); break;
		case ksett_SaveBufferContents : onoff2string(fsett_SaveBufferContents, text); break;

		//output
		case kop_Mix :					long2string((long)(100.0f*fop_Mix), text); break;
		case kop_Level :				dB2string(fop_Level, text); break;
	}
}

//----------------------------------------------------------------------------
void BufferSynth2::getParameterName(long index, char *label)
{
	switch (index)
	{
		//buffer 1
		case kb1_Start :				strcpy(label, "Buffer1 Start"); break;
		case kb1_End :					strcpy(label, "Buffer1 End"); break;
		case kb1_Size :					strcpy(label, "Buffer1 Size"); break;
		case kb1_SizeFrom :				strcpy(label, "Buffer1 Size From"); break;
		case kb1_RetainSize :			strcpy(label, "Buffer1 Lock Size"); break;
		case kb1_Size2Tempo :			strcpy(label, "Buffer1 Size2Tempo"); break;
		case kb1_RecThreshold :			strcpy(label, "Buffer1 RecThresh"); break;
		case kb1_Speed_Pitch :
			if(fsett_SynthMode < 0.5f)
				strcpy(label, "Buffer1 Speed");
			else
				strcpy(label, "Buffer1 Pitch");
			break;
		case kb1_Level :				strcpy(label, "Buffer1 Level"); break;
		case kb1_Input :				strcpy(label, "Buffer1 Input"); break;
		case kb1_StretchFile :			strcpy(label, "Buffer1 Stretch"); break;
		case kb1_LinearInterp :			strcpy(label, "Buffer1 L.Interp"); break;
		case kb1_Reverse :				strcpy(label, "Buffer1 Reverse"); break;
		case kb1_OnlyOPWhenFrozen :		strcpy(label, "Buffer1 Unmute"); break;
		case kb1_MIDINotesSetFreeze :	strcpy(label, "Buffer1 MIDI Freezes"); break;
		case kb1_Freeze :				strcpy(label, "Buffer1 Freeze"); break;
		case kb1_SizeLessThanMaxFreezes:strcpy(label, "Buffer1 Size<Max"); break;
		case kb1_InvertSize :			strcpy(label, "Buffer1 Invert Size"); break;
		case kb1_ReadPosition :			strcpy(label, "Buffer1 ReadPos"); break;
		case kb1_ResetRPOnMIDINote :	strcpy(label, "Buffer1 ResetOnNote"); break;
		case kb1_Pan :					strcpy(label, "Buffer1 Pan"); break;
		case kb1_IPGain :				strcpy(label, "Buffer1 IP Gain"); break;

		//buffer 2
		case kb2_Start :				strcpy(label, "Buffer2 Start"); break;
		case kb2_End :					strcpy(label, "Buffer2 End"); break;
		case kb2_Size :					strcpy(label, "Buffer2 Size"); break;
		case kb2_SizeFrom :				strcpy(label, "Buffer2 Size From"); break;
		case kb2_RetainSize :			strcpy(label, "Buffer2 Lock Size"); break;
		case kb2_Size2Tempo :			strcpy(label, "Buffer2 Size2Tempo"); break;
		case kb2_RecThreshold :			strcpy(label, "Buffer2 RecThresh"); break;
		case kb2_Speed_Pitch :
			if(fsett_SynthMode < 0.5f)
				strcpy(label, "Buffer2 Speed");
			else
				strcpy(label, "Buffer2 Pitch");
			break;
		case kb2_Level :				strcpy(label, "Buffer2 Level"); break;
		case kb2_Input :				strcpy(label, "Buffer2 Input"); break;
		case kb2_StretchFile :			strcpy(label, "Buffer2 Stretch"); break;
		case kb2_LinearInterp :			strcpy(label, "Buffer2 L.Interp"); break;
		case kb2_Reverse :				strcpy(label, "Buffer2 Reverse"); break;
		case kb2_OnlyOPWhenFrozen :		strcpy(label, "Buffer2 Unmute"); break;
		case kb2_MIDINotesSetFreeze :	strcpy(label, "Buffer2 MIDI Freezes"); break;
		case kb2_Freeze :				strcpy(label, "Buffer2 Freeze"); break;
		case kb2_ModDestination :		strcpy(label, "Buffer2 Mod Dest"); break;
		case kb2_ModDepth :				strcpy(label, "Buffer2 Mod Depth"); break;
		case kb2_SizeLessThanMaxFreezes:strcpy(label, "Buffer2 Size<Max"); break;
		case kb2_InvertSize :			strcpy(label, "Buffer2 Invert Size"); break;
		case kb2_Envelope :				strcpy(label, "Buffer2 Envelope"); break;
		case kb2_ReadPosition :			strcpy(label, "Buffer2 ReadPos"); break;
		case kb2_ResetRPOnMIDINote :	strcpy(label, "Buffer2 ResetOnNote"); break;
		case kb2_Pan :					strcpy(label, "Buffer2 Pan"); break;
		case kb2_IPGain :				strcpy(label, "Buffer2 IPGain"); break;

		//amplitude envelope
		case kae_OnOff :				strcpy(label, "AmpEnv On/Off"); break;
		case kae_Attack :				strcpy(label, "AmpEnv Attack"); break;
		case kae_Decay :				strcpy(label, "AmpEnv Decay"); break;
		case kae_Sustain :				strcpy(label, "AmpEnv Sustain"); break;
		case kae_Release :				strcpy(label, "AmpEnv Release"); break;
		case kae_SegmentTime :			strcpy(label, "AmpEnv SegTime"); break;
		/*case kae_FreezeTriggers :		strcpy(label, "aFrzTrig"); break;
		case kae_MIDINotesTrigger :		strcpy(label, "aMIDITrg"); break;*/

		//second envelope
		case ke2_Attack :				strcpy(label, "Env2 Attack"); break;
		case ke2_Decay :				strcpy(label, "Env2 Decay"); break;
		case ke2_Sustain :				strcpy(label, "Env2 Sustain"); break;
		case ke2_Release :				strcpy(label, "Env2 Release"); break;
		case ke2_SegmentTime :			strcpy(label, "Env2 SegTime"); break;
		case ke2_MIDINotesTrigger :		strcpy(label, "Env2 Note Triggers"); break;
		case ke2_BarStartTriggers :		strcpy(label, "Env2 BS Triggers"); break;
		case ke2_Destination :			strcpy(label, "Env2 Destination"); break;
		case ke2_Direction :			strcpy(label, "Env2 Invert"); break;
		case ke2_ModDepth :				strcpy(label, "Env2 Mod Depth"); break;

		//LFO 1
		case klfo1_Freq_Note :			strcpy(label, "LFO1 Period"); break;
		case klfo1_TempoSync :			strcpy(label, "LFO1 Tempo"); break;
		case klfo1_Waveform :			strcpy(label, "LFO1 Waveform"); break;
		case klfo1_BarStartResets :		strcpy(label, "LFO1 BS Resets"); break;
		case klfo1_MIDINotesReset :		strcpy(label, "LFO1 Note Resets"); break;
		case klfo1_Destination :		strcpy(label, "LFO1 Destination"); break;
		case klfo1_ModDepth :			strcpy(label, "LFO1 ModDpth"); break;

		//LFO 2
		case klfo2_Freq_Note :			strcpy(label, "LFO2 Period"); break;
		case klfo2_TempoSync :			strcpy(label, "LFO2 Tempo"); break;
		case klfo2_Waveform :			strcpy(label, "LFO2 Waveform"); break;
		case klfo2_BarStartResets :		strcpy(label, "LFO2 BS Resets"); break;
		case klfo2_MIDINotesReset :		strcpy(label, "LFO2 Note Resets"); break;
		case klfo2_Destination :		strcpy(label, "LFO2 Destination"); break;
		case klfo2_ModDepth :			strcpy(label, "LFO2 Mod Depth"); break;

		//filter
		case kfilt_OnOff :				strcpy(label, "Filter OnOff"); break;
		case kfilt_Cutoff :				strcpy(label, "Filter Cutoff"); break;
		case kfilt_Resonance :			strcpy(label, "Filter Res"); break;
		case kfilt_Type :				strcpy(label, "Filter Type"); break;

		//settings
		case ksett_MIDILearn :			strcpy(label, "MIDI Learn"); break;
		case ksett_SynthMode :			strcpy(label, "Synth Mode"); break;
		case ksett_PolyphonicMode :		strcpy(label, "Polyphonic Mode"); break;
		case ksett_PitchCorrection :	strcpy(label, "Pitch Corrrection"); break;
		case ksett_SaveBufferContents : strcpy(label, "Save Buffer Contents"); break;

		//output
		case kop_Mix :					strcpy(label, "Output Mix"); break;
		case kop_Level :				 strcpy(label, "Output Level"); break;
	}
}

//----------------------------------------------------------------------------
float BufferSynth2::getCC(long index)
{
	float retval;

	switch(index)
	{
		case kb1_Start :				retval = (float)pd->presets[curProgram].b1_Start.MIDICC; break;
		case kb1_End :					retval = (float)pd->presets[curProgram].b1_End.MIDICC; break;
		case kb1_Size :					retval = (float)pd->presets[curProgram].b1_Size.MIDICC; break;
		case kb1_SizeFrom :				retval = (float)pd->presets[curProgram].b1_SizeFrom.MIDICC; break;
		case kb1_RetainSize :			retval = (float)pd->presets[curProgram].b1_RetainSize.MIDICC; break;
		case kb1_Size2Tempo :			retval = (float)pd->presets[curProgram].b1_Size2Tempo.MIDICC; break;
		case kb1_RecThreshold :			retval = (float)pd->presets[curProgram].b1_RecThreshold.MIDICC; break;
		case kb1_Speed_Pitch :			retval = (float)pd->presets[curProgram].b1_Speed_Pitch.MIDICC; break;
		case kb1_Level :				retval = (float)pd->presets[curProgram].b1_Level.MIDICC; break;
		case kb1_Input :				retval = (float)pd->presets[curProgram].b1_Input.MIDICC; break;
		case kb1_StretchFile :			retval = (float)pd->presets[curProgram].b1_StretchFile.MIDICC; break;
		case kb1_LinearInterp :			retval = (float)pd->presets[curProgram].b1_LinearInterp.MIDICC; break;
		case kb1_Reverse :				retval = (float)pd->presets[curProgram].b1_Reverse.MIDICC; break;
		case kb1_OnlyOPWhenFrozen :		retval = (float)pd->presets[curProgram].b1_OnlyOPWhenFrozen.MIDICC; break;
		case kb1_MIDINotesSetFreeze :	retval = (float)pd->presets[curProgram].b1_MIDINotesSetFreeze.MIDICC; break;
		case kb1_Freeze :				retval = (float)pd->presets[curProgram].b1_Freeze.MIDICC; break;
		case kb1_SizeLessThanMaxFreezes:retval = (float)pd->presets[curProgram].b1_SizeLessThanMaxFreezes.MIDICC; break;
		case kb1_InvertSize :			retval = (float)pd->presets[curProgram].b1_InvertSize.MIDICC; break;
		case kb1_ReadPosition :			retval = (float)pd->presets[curProgram].b1_ReadPosition.MIDICC; break;
		case kb1_ResetRPOnMIDINote :	retval = (float)pd->presets[curProgram].b1_ResetRPOnMIDINote.MIDICC; break;
		case kb1_Pan :					retval = (float)pd->presets[curProgram].b1_Pan.MIDICC; break;
		case kb1_IPGain :				retval = (float)pd->presets[curProgram].b1_IPGain.MIDICC; break;

		//buffer 2
		case kb2_Start :				retval = (float)pd->presets[curProgram].b2_Start.MIDICC; break;
		case kb2_End :					retval = (float)pd->presets[curProgram].b2_End.MIDICC; break;
		case kb2_Size :					retval = (float)pd->presets[curProgram].b2_Size.MIDICC; break;
		case kb2_SizeFrom :				retval = (float)pd->presets[curProgram].b2_SizeFrom.MIDICC; break;
		case kb2_RetainSize :			retval = (float)pd->presets[curProgram].b2_RetainSize.MIDICC; break;
		case kb2_Size2Tempo :			retval = (float)pd->presets[curProgram].b2_Size2Tempo.MIDICC; break;
		case kb2_RecThreshold :			retval = (float)pd->presets[curProgram].b2_RecThreshold.MIDICC; break;
		case kb2_Speed_Pitch :			retval = (float)pd->presets[curProgram].b2_Speed_Pitch.MIDICC; break;
		case kb2_Level :				retval = (float)pd->presets[curProgram].b2_Level.MIDICC; break;
		case kb2_Input :				retval = (float)pd->presets[curProgram].b2_Input.MIDICC; break;
		case kb2_StretchFile :			retval = (float)pd->presets[curProgram].b2_StretchFile.MIDICC; break;
		case kb2_LinearInterp :			retval = (float)pd->presets[curProgram].b2_LinearInterp.MIDICC; break;
		case kb2_Reverse :				retval = (float)pd->presets[curProgram].b2_Reverse.MIDICC; break;
		case kb2_OnlyOPWhenFrozen :		retval = (float)pd->presets[curProgram].b2_OnlyOPWhenFrozen.MIDICC; break;
		case kb2_MIDINotesSetFreeze :	retval = (float)pd->presets[curProgram].b2_MIDINotesSetFreeze.MIDICC; break;
		case kb2_Freeze :				retval = (float)pd->presets[curProgram].b2_Freeze.MIDICC; break;
		case kb2_ModDestination :		retval = (float)pd->presets[curProgram].b2_ModDestination.MIDICC; break;
		case kb2_ModDepth :				retval = (float)pd->presets[curProgram].b2_ModDepth.MIDICC; break;
		case kb2_SizeLessThanMaxFreezes:retval = (float)pd->presets[curProgram].b2_SizeLessThanMaxFreezes.MIDICC; break;
		case kb2_InvertSize :			retval = (float)pd->presets[curProgram].b2_InvertSize.MIDICC; break;
		case kb2_Envelope :				retval = (float)pd->presets[curProgram].b2_Envelope.MIDICC; break;
		case kb2_ReadPosition :			retval = (float)pd->presets[curProgram].b2_ReadPosition.MIDICC; break;
		case kb2_ResetRPOnMIDINote :	retval = (float)pd->presets[curProgram].b2_ResetRPOnMIDINote.MIDICC; break;
		case kb2_Pan :					retval = (float)pd->presets[curProgram].b2_Pan.MIDICC; break;
		case kb2_IPGain :				retval = (float)pd->presets[curProgram].b2_IPGain.MIDICC; break;

		//amplitude envelope
		case kae_OnOff :				retval = (float)pd->presets[curProgram].ae_OnOff.MIDICC; break;
		case kae_Attack :				retval = (float)pd->presets[curProgram].ae_Attack.MIDICC; break;
		case kae_Decay :				retval = (float)pd->presets[curProgram].ae_Decay.MIDICC; break;
		case kae_Sustain :				retval = (float)pd->presets[curProgram].ae_Sustain.MIDICC; break;
		case kae_Release :				retval = (float)pd->presets[curProgram].ae_Release.MIDICC; break;
		case kae_SegmentTime :			retval = (float)pd->presets[curProgram].ae_SegmentTime.MIDICC; break;
		/*case kae_FreezeTriggers :		retval = (float)pd->presets[curProgram].ae_FreezeTriggers.MIDICC; break;
		case kae_MIDINotesTrigger :		retval = (float)pd->presets[curProgram].ae_MIDINotesTrigger.MIDICC; break;*/

		//second envelope
		case ke2_Attack :				retval = (float)pd->presets[curProgram].e2_Attack.MIDICC; break;
		case ke2_Decay :				retval = (float)pd->presets[curProgram].e2_Decay.MIDICC; break;
		case ke2_Sustain :				retval = (float)pd->presets[curProgram].e2_Sustain.MIDICC; break;
		case ke2_Release :				retval = (float)pd->presets[curProgram].e2_Release.MIDICC; break;
		case ke2_SegmentTime :			retval = (float)pd->presets[curProgram].e2_SegmentTime.MIDICC; break;
		case ke2_MIDINotesTrigger :		retval = (float)pd->presets[curProgram].e2_MIDINotesTrigger.MIDICC; break;
		case ke2_BarStartTriggers :		retval = (float)pd->presets[curProgram].e2_BarStartTriggers.MIDICC; break;
		case ke2_Destination :			retval = (float)pd->presets[curProgram].e2_Destination.MIDICC; break;
		case ke2_Direction :			retval = (float)pd->presets[curProgram].e2_Direction.MIDICC; break;
		case ke2_ModDepth :				retval = (float)pd->presets[curProgram].e2_ModDepth.MIDICC; break;

		//LFO1
		case klfo1_Freq_Note :			retval = (float)pd->presets[curProgram].lfo1_Freq_Note.MIDICC; break;
		case klfo1_TempoSync :			retval = (float)pd->presets[curProgram].lfo1_TempoSync.MIDICC; break;
		case klfo1_Waveform :			retval = (float)pd->presets[curProgram].lfo1_Waveform.MIDICC; break;
		case klfo1_BarStartResets :		retval = (float)pd->presets[curProgram].lfo1_BarStartResets.MIDICC; break;
		case klfo1_MIDINotesReset :		retval = (float)pd->presets[curProgram].lfo1_MIDINotesReset.MIDICC; break;
		case klfo1_Destination :		retval = (float)pd->presets[curProgram].lfo1_Destination.MIDICC; break;
		case klfo1_ModDepth :			retval = (float)pd->presets[curProgram].lfo1_ModDepth.MIDICC; break;

		//LFO2
		case klfo2_Freq_Note :			retval = (float)pd->presets[curProgram].lfo2_Freq_Note.MIDICC; break;
		case klfo2_TempoSync :			retval = (float)pd->presets[curProgram].lfo2_TempoSync.MIDICC; break;
		case klfo2_Waveform :			retval = (float)pd->presets[curProgram].lfo2_Waveform.MIDICC; break;
		case klfo2_BarStartResets :		retval = (float)pd->presets[curProgram].lfo2_BarStartResets.MIDICC; break;
		case klfo2_MIDINotesReset :		retval = (float)pd->presets[curProgram].lfo2_MIDINotesReset.MIDICC; break;
		case klfo2_Destination :		retval = (float)pd->presets[curProgram].lfo2_Destination.MIDICC; break;
		case klfo2_ModDepth :			retval = (float)pd->presets[curProgram].lfo2_ModDepth.MIDICC; break;

		//filter
		case kfilt_OnOff :				retval = (float)pd->presets[curProgram].filt_OnOff.MIDICC; break;
		case kfilt_Cutoff :				retval = (float)pd->presets[curProgram].filt_Cutoff.MIDICC; break;
		case kfilt_Resonance :			retval = (float)pd->presets[curProgram].filt_Resonance.MIDICC; break;
		case kfilt_Type :				retval = (float)pd->presets[curProgram].filt_Type.MIDICC; break;

		//settings
		case ksett_MIDILearn :			retval = (float)pd->presets[curProgram].sett_MIDILearn.MIDICC; break;
		case ksett_SynthMode :			retval = (float)pd->presets[curProgram].sett_SynthMode.MIDICC; break;
		case ksett_PolyphonicMode :		retval = (float)pd->presets[curProgram].sett_PolyphonicMode.MIDICC; break;
		case ksett_PitchCorrection :	retval = (float)pd->presets[curProgram].sett_PitchCorrection.MIDICC; break;
		case ksett_SaveBufferContents : retval = (float)pd->presets[curProgram].sett_SaveBufferContents.MIDICC; break;

		//output
		case kop_Mix :					retval = (float)pd->presets[curProgram].op_Mix.MIDICC; break;
		case kop_Level :				retval = (float)pd->presets[curProgram].op_Level.MIDICC; break;
	}

	return retval;
}

//----------------------------------------------------------------------------
//tells the host what the plugin can do: 1=can do, -1=cannot do, 0=don't know
long BufferSynth2::canDo(char *text)
{
    //return -1;

	if(!strcmp(text, "sendVstEvents")) return -1;
	if(!strcmp(text, "sendVstMidiEvent")) return -1;
	if(!strcmp(text, "sendVstTimeInfo")) return -1;
	if(!strcmp(text, "receiveVstEvents")) return 1;			//* - need for processEvents
	if(!strcmp(text, "receiveVstMidiEvent")) return 1;		//* - need for MIDI handling
	if(!strcmp(text, "receiveVstTimeInfo")) return 1;
	if(!strcmp(text, "offline")) return -1;
	if(!strcmp(text, "plugAsChannelInsert")) return -1;
	if(!strcmp(text, "plugAsSend")) return -1;
	if(!strcmp(text, "mixDryWet")) return -1;
	if(!strcmp(text, "noRealTime")) return -1;
	if(!strcmp(text, "multipass")) return -1;
	if(!strcmp(text, "metapass")) return -1;
	if(!strcmp(text, "1in1out")) return -1;
	if(!strcmp(text, "1in2out")) return -1;
	if(!strcmp(text, "2in1out")) return -1;
	if(!strcmp(text, "2in2out")) return -1;
	if(!strcmp(text, "2in4out")) return -1;
	if(!strcmp(text, "4in2out")) return -1;
	if(!strcmp(text, "4in4out")) return -1;
	if(!strcmp(text, "4in8out")) return -1;					// 4:2 matrix to surround bus
	if(!strcmp(text, "8in4out")) return -1;					// surround bus to 4:2 matrix
	if(!strcmp(text, "8in8out")) return -1;
	if(!strcmp(text, "midiProgramNames")) return -1;
	if(!strcmp(text, "conformsToWindowRules") ) return -1;	// mac: doesn't mess with grafport. general: may want
															// to call sizeWindow (). if you want to use sizeWindow (),
															// you must return true (1) in canDo ("conformsToWindowRules")
	if(!strcmp(text, "bypass")) return -1;

	return -1;
}

//----------------------------------------------------------------------------
//don't know how this works - I've never used it
//----------------------------------------------------------------------------
float BufferSynth2::getVu()
{
    float cvu = vu;
	
	vu = 0;
	return cvu;
}

//----------------------------------------------------------------------------
bool BufferSynth2::getEffectName (char* name)
{
    strcpy(name,kEffectName);
    return true;
}

//----------------------------------------------------------------------------
bool BufferSynth2::getVendorString (char* text)
{
    strcpy(text, kVendor);
    return true;
}

//----------------------------------------------------------------------------
bool BufferSynth2::getProductString (char* text)
{
    strcpy(text, kProduct);
    return true;
}

//----------------------------------------------------------------------------
long BufferSynth2::getVendorVersion ()
{
    return kVersionNo;
}

//----------------------------------------------------------------------------
VstPlugCategory BufferSynth2::getPlugCategory()
{
    return (kPlugCategEffect);  //****Remember to change if synth etc.****
}

//----------------------------------------------------------------------------
bool BufferSynth2::getInputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	if(index == 0)
	{
		sprintf(properties->label, kEffectName, " Left Input", index + 1);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, kEffectName, " Right Input", index + 1);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return returnCode;
}

//----------------------------------------------------------------------------
bool BufferSynth2::getOutputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	if (index == 0)
	{
		sprintf (properties->label, kEffectName, " Left Output", index + 1);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else 
	if (index == 1)
	{
		sprintf (properties->label, kEffectName, " Right Output", index + 1);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return (returnCode);
}

//----------------------------------------------------------------------------
long BufferSynth2::getTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void BufferSynth2::onoff2string(float value, char *text)
{
	if(value < 0.5)
		strcpy(text, "Off");
	else
		strcpy(text, "On");
}

//----------------------------------------------------------------------------
void BufferSynth2::sizefrom2string(float value, char *text)
{
	if(value < 0.5)
		strcpy(text, "Start");
	else
		strcpy(text, "End");
}

//----------------------------------------------------------------------------
void BufferSynth2::size2tempo2string(float value, char *text)
{
	if(value < (1.0f/13.0f))
		strcpy(text, "---");
	else if(value < (2.0f/13.0f))
		strcpy(text, "1/16");
	else if(value < (3.0f/13.0f))
		strcpy(text, "1/8");
	else if(value < (4.0f/13.0f))
		strcpy(text, "1/6");
	else if(value < (5.0f/13.0f))
		strcpy(text, "1/4");
	else if(value < (6.0f/13.0f))
		strcpy(text, "1/3");
	else if(value < (7.0f/13.0f))
		strcpy(text, "1/2");
	else if(value < (8.0f/13.0f))
		strcpy(text, "1");
	else if(value < (9.0f/13.0f))
		strcpy(text, "2");
	else if(value < (10.0f/13.0f))
		strcpy(text, "3");
	else if(value < (11.0f/13.0f))
		strcpy(text, "4");
	else if(value < (12.0f/13.0f))
		strcpy(text, "6");
	else
		strcpy(text, "8");
}

//----------------------------------------------------------------------------
void BufferSynth2::input2string(float value, char *text)
{
	if(value < 0.33f)
		strcpy(text, "Left");
	else if(value < 0.66f)
		strcpy(text, "Right");
	else
		strcpy(text, "Wave");
}

//----------------------------------------------------------------------------
void BufferSynth2::float2string(float value, char *string)
{
	sprintf(string, "%.3f", value);
}

//----------------------------------------------------------------------------
void BufferSynth2::b2dest2string(float value, char *text)
{
	if(value < (1.0f/10.0f))
		strcpy(text, "off");
	else if(value < (2.0f/10.0f))
		strcpy(text, "b1Lvl");
	else if(value < (3.0f/10.0f))
		strcpy(text, "b1Spd");
	else if(value < (4.0f/10.0f))
		strcpy(text, "b1Size");
	else if(value < (5.0f/10.0f))
		strcpy(text, "b1Start");
	else if(value < (6.0f/10.0f))
		strcpy(text, "b1End");
	else if(value < (7.0f/10.0f))
		strcpy(text, "Cutoff");
	else if(value < (8.0f/10.0f))
		strcpy(text, "Res");
	else if(value < (9.0f/10.0f))
		strcpy(text, "b1Pan");
	else
		strcpy(text, "b1RPos");
}

//----------------------------------------------------------------------------
void BufferSynth2::e2dest2string(float value, char *text)
{
	if(value < (1.0f/22.0f))
		strcpy(text, "off");
	else if(value < (2.0f/22.0f))
		strcpy(text, "b1Lvl");
	else if(value < (3.0f/22.0f))
		strcpy(text, "b1Spd");
	else if(value < (4.0f/22.0f))
		strcpy(text, "b1Size");
	else if(value < (5.0f/22.0f))
		strcpy(text, "b1Start");
	else if(value < (6.0f/22.0f))
		strcpy(text, "b1End");
	else if(value < (7.0f/22.0f))
		strcpy(text, "b2Lvl");
	else if(value < (8.0f/22.0f))
		strcpy(text, "b2Spd");
	else if(value < (9.0f/22.0f))
		strcpy(text, "b2Size");
	else if(value < (10.0f/22.0f))
		strcpy(text, "b2Start");
	else if(value < (11.0f/22.0f))
		strcpy(text, "b2End");
	else if(value < (12.0f/22.0f))
		strcpy(text, "Cutoff");
	else if(value < (13.0f/22.0f))
		strcpy(text, "Res");
	else if(value < (14.0f/22.0f))
		strcpy(text, "l1Depth");
	else if(value < (15.0f/22.0f))
		strcpy(text, "l2Depth");
	else if(value < (16.0f/22.0f))
		strcpy(text, "b1Pan");
	else if(value < (17.0f/22.0f))
		strcpy(text, "b1RPos");
	else if(value < (18.0f/22.0f))
		strcpy(text, "b2Pan");
	else if(value < (19.0f/22.0f))
		strcpy(text, "b2RPos");
	else if(value < (20.0f/22.0f))
		strcpy(text, "b2Dpth");
	else if(value < (21.0f/22.0f))
		strcpy(text, "l1Freq");
	else
		strcpy(text, "l2Freq");
}

//----------------------------------------------------------------------------
void BufferSynth2::e2invert2string(float value, char *text)
{
	if(value < 0.5)
		strcpy(text, "off");
	else
		strcpy(text, "invert");
}

//----------------------------------------------------------------------------
void BufferSynth2::lfonote2string(float value, char *text)
{
	if(value < (1.0f/12.0f))
		strcpy(text, "1/16");
	else if(value < (2.0f/12.0f))
		strcpy(text, "1/8");
	else if(value < (3.0f/12.0f))
		strcpy(text, "1/6");
	else if(value < (4.0f/12.0f))
		strcpy(text, "1/4");
	else if(value < (5.0f/12.0f))
		strcpy(text, "1/3");
	else if(value < (6.0f/12.0f))
		strcpy(text, "1/2");
	else if(value < (7.0f/12.0f))
		strcpy(text, "1");
	else if(value < (8.0f/12.0f))
		strcpy(text, "2");
	else if(value < (9.0f/12.0f))
		strcpy(text, "3");
	else if(value < (10.0f/12.0f))
		strcpy(text, "4");
	else if(value < (11.0f/12.0f))
		strcpy(text, "6");
	else
		strcpy(text, "8");
}

//----------------------------------------------------------------------------
void BufferSynth2::lfodest2string(float value, char *text)
{
	if(value < (1.0f/18.0f))
		strcpy(text, "off");
	else if(value < (2.0f/18.0f))
		strcpy(text, "b1Lvl");
	else if(value < (3.0f/18.0f))
		strcpy(text, "b1Spd");
	else if(value < (4.0f/18.0f))
		strcpy(text, "b1Size");
	else if(value < (5.0f/18.0f))
		strcpy(text, "b1Start");
	else if(value < (6.0f/18.0f))
		strcpy(text, "b1End");
	else if(value < (7.0f/18.0f))
		strcpy(text, "b2Lvl");
	else if(value < (8.0f/18.0f))
		strcpy(text, "b2Spd");
	else if(value < (9.0f/18.0f))
		strcpy(text, "b2Size");
	else if(value < (10.0f/18.0f))
		strcpy(text, "b2Start");
	else if(value < (11.0f/18.0f))
		strcpy(text, "b2End");
	else if(value < (12.0f/18.0f))
		strcpy(text, "Cutoff");
	else if(value < (13.0f/18.0f))
		strcpy(text, "Res");
	else if(value < (14.0f/18.0f))
		strcpy(text, "b1Pan");
	else if(value < (15.0f/18.0f))
		strcpy(text, "b1RPos");
	else if(value < (16.0f/18.0f))
		strcpy(text, "b2Pan");
	else if(value < (17.0f/18.0f))
		strcpy(text, "b2RPos");
	else
		strcpy(text, "b2Dpth");
}

//----------------------------------------------------------------------------
void BufferSynth2::temposync2string(float value, char *text)
{
	if(value < 0.5)
		strcpy(text, "Hz");
	else
		strcpy(text, "Beat");
}

//----------------------------------------------------------------------------
void BufferSynth2::wave2string(float value, char *text)
{
	if(value < (1.0f/5.0f))
		strcpy(text, "Sine");
	else if(value < (2.0f/5.0f))
		strcpy(text, "Saw");
	else if(value < (3.0f/5.0f))
		strcpy(text, "Squ");
	else if(value < (4.0f/5.0f))
		strcpy(text, "S/H");
	else
		strcpy(text, "Ramp");
}

//----------------------------------------------------------------------------
void BufferSynth2::filttype2string(float value, char *text)
{
	if(value < 0.33f)
		strcpy(text, "Hi");
	else if(value < 0.66f)
		strcpy(text, "Band");
	else
		strcpy(text, "Low");
}

//----------------------------------------------------------------------------
void BufferSynth2::pcorrection2string(float value, char *text)
{
	if(value < 0.33f)
		strcpy(text, "off");
	else if(value < 0.66f)
		strcpy(text, "Brutal");
	else
		strcpy(text, "Nice");
}
