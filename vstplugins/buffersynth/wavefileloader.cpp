//	wavefileloader.cpp - Class for loading .wav files.
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

#include <string.h>
#include <stdio.h>
#include "wavefileloader.h"

#include "EndianSwapFunctions.h"

float *wavefileloader::LoadWaveFile(char *filename, float samplerate)
{
	unsigned long i/*, j*/;
	bool is16bit;
	short *t_s_p;//temp_short_pointer
	unsigned char *t_uc_p;//ditto
	bool test;

	float *buffer;

	WAVEFILE wfile;
	FILE *fp;

	fp = NULL;

	test = false;
        
#ifndef MAC
	fp = fopen(filename, "rb");
#else
        char *tempfilename;
        //fp = fopen("/Users/niallmoody/Desktop/TestLoop.wav", "rb");
        tempfilename = filename+12;
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

	if(!fp)
        {
		test = true;//return NULL;
                    fp = fopen("Aargh.txt", "w");
                    fprintf(fp, "Aargh! 1");
                    fclose(fp);
        }
	if(!test)
	{
		fread(&wfile.Riff, 1, 4, fp);
		fread(&wfile.RiffSize, 4, 1, fp);
		fread(&wfile.Wave, 1, 4, fp);	
		fread(&wfile.Fmt, 1, 4, fp);
		fread(&wfile.FmtSize, 4, 1, fp);
		fread(&wfile.FormatTag, 2, 1, fp);
		fread(&wfile.noChannels, 2, 1, fp);
		fread(&wfile.Samplerate, 4, 1, fp);
		fread(&wfile.AvgBytesPerSec, 4, 1, fp);
		fread(&wfile.BlockAlign, 2, 1, fp);
		fread(&wfile.BitDepth, 2, 1, fp);
		fread(&wfile.Data, 1, 4, fp);
		fread(&wfile.DataSize, 4, 1, fp);

		wfile.RiffSize = LittleLong(wfile.RiffSize);
		wfile.FmtSize = LittleLong(wfile.FmtSize);
		wfile.FormatTag = LittleShort(wfile.FormatTag);
		wfile.noChannels = LittleShort(wfile.noChannels);
		wfile.Samplerate = LittleLong(wfile.Samplerate);
		wfile.AvgBytesPerSec = LittleLong(wfile.AvgBytesPerSec);
		wfile.BlockAlign = LittleShort(wfile.BlockAlign);
		wfile.BitDepth = LittleShort(wfile.BitDepth);
		wfile.DataSize = LittleLong(wfile.DataSize);

		if(!((wfile.Riff[0]&&'R')&&(wfile.Riff[1]&&'I')&&(wfile.Riff[2]&&'F')&&(wfile.Riff[3]&&'F')))
		{
			if(fp)
                        {
				fclose(fp);
                                fp = fopen("Aargh.txt", "w");
                                fprintf(fp, "Aargh! 2");
                                fclose(fp);
                        }
			return NULL;
		}
		if(!((wfile.Wave[0]&&'W')&&(wfile.Wave[1]&&'A')&&(wfile.Wave[2]&&'V')&&(wfile.Wave[3]&&'E')))
		{
			if(fp)
                        {
				fclose(fp);
                                fp = fopen("Aargh.txt", "w");
                                fprintf(fp, "Aargh! 3");
                                fclose(fp);
                        }
			return NULL;
		}
		if(!((wfile.Fmt[0]&&'f')&&(wfile.Fmt[1]&&'m')&&(wfile.Fmt[2]&&'t')&&(wfile.Fmt[3]&&' ')))
		{
			if(fp)
                        {
				fclose(fp);
                                fp = fopen("Aargh.txt", "w");
                                fprintf(fp, "Aargh! 4");
                                fclose(fp);
                        }
			return NULL;
		}
		if(!((wfile.Data[0]&&'d')&&(wfile.Data[1]&&'a')&&(wfile.Data[2]&&'t')&&(wfile.Data[3]&&'a')))
		{
			if(fp)
                        {
				fclose(fp);
                                fp = fopen("Aargh.txt", "w");
                                fprintf(fp, "Aargh! 5");
                                fclose(fp);
                        }
			return NULL;
		}
		if(wfile.FormatTag != 1)
		{
			if(fp)
                        {
				fclose(fp);
                                fp = fopen("Aargh.txt", "w");
                                fprintf(fp, "Aargh! 6");
                                fclose(fp);
                        }
			return NULL;
		}

		if(wfile.BitDepth <= 16)
		{
			size = wfile.DataSize/wfile.BlockAlign;

			if(wfile.BitDepth == 16)
			{
				is16bit = true;
				wfile.waveformData = new short[size];

				t_s_p = (short *)wfile.waveformData;

				if(wfile.noChannels == 1)
				{
					for(i=0;i<size;i++)
					{
						fread((t_s_p+i), 2, 1, fp);
						t_s_p[i] = LittleShort(t_s_p[i]);
					}
				}
				else if(wfile.noChannels == 2)
				{
					for(i=0;i<size;i++)
					{
						fread((t_s_p+i), 2, 1, fp);
						t_s_p[i] = LittleShort(t_s_p[i]);
						fseek(fp, 2, SEEK_CUR);
					}
				}
				else
				{
					if(fp)
                                        {
                                                fclose(fp);
                                                fp = fopen("Aargh.txt", "w");
                                                fprintf(fp, "Aargh! 7");
                                                fclose(fp);
                                        }
					return NULL;
				}
			}
			else
			{
				is16bit = false;
				wfile.waveformData = new unsigned char[size];

				t_uc_p = (unsigned char *)wfile.waveformData;


				if(wfile.noChannels == 1)
				{
					for(i=0;i<size;i++)
						fread((t_uc_p+i), 1, 1, fp);
				}
				else if(wfile.noChannels == 2)
				{
					for(i=0;i<size;i++)
					{
						fread((t_uc_p+i), 2, 1, fp);
						fseek(fp, 1, SEEK_CUR);
					}
				}
				else
				{
					if(fp)
                                        {
                                                fclose(fp);
                                                fp = fopen("Aargh.txt", "w");
                                                fprintf(fp, "Aargh! 8");
                                                fclose(fp);
                                        }
					return NULL;
				}
			}
		}
		else
		{
			if(fp)
                        {
				fclose(fp);
                                fp = fopen("Aargh.txt", "w");
                                fprintf(fp, "Aargh! 9");
                                fclose(fp);
                        }
			return NULL;
		}

		buffer = new float[size];

		if(is16bit)
			t_s_p = (short *)wfile.waveformData;
		else
			t_uc_p = (unsigned char *)wfile.waveformData;

		for(i=0;i<size;i++)
		{
			if(is16bit)
				buffer[i] = ((float)t_s_p[i]/32767.0f);
			else
				buffer[i] = (((float)t_uc_p[i]-127.0f)/127.0f);
		}

		if((float)wfile.Samplerate != samplerate)
			buffer = AlterSamplerate(buffer, (float)wfile.Samplerate, samplerate);

		/*if(fp)
                {
                        fclose(fp);
                        fp = fopen("Aargh.txt", "w");
                        fprintf(fp, "Aargh! (altersamplerate)");
                        fclose(fp);
                }*/
	}

	/*fp = fopen("TM2.log", "w");
	if(!fp)
		return NULL;*/

	if(test)
	{
		/*fprintf(fp, "Oh Dear\n");
		fprintf(fp, filename);
		fclose(fp);*/
		return NULL;
	}

	/*fprintf(fp, filename);
	fprintf(fp, "\nRiff : %c%c%c%c\n", wfile.Riff[0], wfile.Riff[1], wfile.Riff[2], wfile.Riff[3]);
	fprintf(fp, "Riff Size : %u\n", wfile.RiffSize);
	fprintf(fp, "Wave : %c%c%c%c\n", wfile.Wave[0], wfile.Wave[1], wfile.Wave[2], wfile.Wave[3]);
	fprintf(fp, "Fmt : %c%c%c%c\n", wfile.Fmt[0], wfile.Fmt[1], wfile.Fmt[2], wfile.Fmt[3]);
	fprintf(fp, "Format Size : %u\n", wfile.FmtSize);
	fprintf(fp, "Format Tag : %i\n", wfile.FormatTag);
	fprintf(fp, "No. Channels : %i\n", wfile.noChannels);
	fprintf(fp, "Samplerate : %u\n", wfile.Samplerate);
	fprintf(fp, "Bytes/Sec : %u\n", wfile.AvgBytesPerSec);
	fprintf(fp, "Block Align : %i\n", wfile.BlockAlign);
	fprintf(fp, "Bit Depth : %i\n", wfile.BitDepth);
	fprintf(fp, "Data : %c%c%c%c\n", wfile.Data[0], wfile.Data[1], wfile.Data[2], wfile.Data[3]);
	fprintf(fp, "Data Size : %u\n", wfile.DataSize);
	fprintf(fp, "wfl.Size : %u\n", size);

	fprintf(fp, "Sample 0 : %f\n", buffer[0]);
	fprintf(fp, "Sample 1 : %f\n", buffer[1]);
	fprintf(fp, "Sample 2 : %f\n", buffer[2]);
	fprintf(fp, "Sample 3 : %f\n", buffer[3]);
	fprintf(fp, "Sample 4 : %f\n", buffer[4]);
	fprintf(fp, "Sample 5 : %f\n", buffer[5]);
	fprintf(fp, "Sample 6 : %f\n", buffer[6]);
	fprintf(fp, "Sample 7 : %f\n", buffer[7]);
	fprintf(fp, "Sample 8 : %f\n", buffer[8]);
	fprintf(fp, "Sample 9 : %f\n", buffer[9]);
	fprintf(fp, "Sample 10 : %f\n", buffer[10]);
	fprintf(fp, "Sample 100 : %f\n", buffer[100]);
	fprintf(fp, "Sample 1000 : %f\n", buffer[1000]);*/

	if(wfile.waveformData)
	{
    	if(is16bit)
			delete [] (short *)wfile.waveformData;
		else
			delete [] (unsigned char *)wfile.waveformData;

		//fprintf(fp, "deleted...");
	}

	if(fp)
		fclose(fp);

	return buffer;
}

unsigned long wavefileloader::getSize()
{
	return size;
}

float *wavefileloader::AlterSamplerate(float *buffer, float sr_src, float sr_dest)
{
	unsigned long i;
	float index, increment;
	float *temp;
	unsigned long x1, x2;

	index = 0.0f;
	increment = sr_src/sr_dest;

	size = (unsigned long)(((float)size/sr_src) * sr_dest);

	temp = new float[size];

	for(i=0;i<size;i++)
	{
		x1 = (int) index;
		x2 = x1 + 1;
		if(x2 >= size)
			x2 = 0;

		temp[i] = calcInterpValue (buffer[x1], buffer[x2], index);

		index += increment;
	}

	delete [] buffer;
	buffer = new float[size];

	for(i=0;i<size;i++)
		buffer[i] = temp[i];
	
	delete [] temp;

	return buffer;
}

float wavefileloader::calcInterpValue(float val1, float val2, float index)
{
	float outval, index_fract;

	index_fract = index - (float)((int)(index));
	outval = val1 + ((val2 - val1)*index_fract);

	return outval;
}
