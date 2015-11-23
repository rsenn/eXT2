/*
  ZynAddSubFX - a software synthesizer

  main.c  -  Main file of the synthesizer
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <FL/x.H>
#include <iostream>

#include "Misc/Master.h"
#include "Misc/Util.h"
#include "Misc/Dump.h"
#include "Input/NULLMidiIn.h"
#include "UI/MasterUI.h"

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "Output/VSTaudiooutput.h"

//--------------------------------------------------------------------------------------
extern Dump dump;
Display* display = 0;

static int instances;

Config config;
REALTYPE *denormalkillbuf;

void* VSTSynth_GuiThread (void *arg);

//--------------------------------------------------------------------------------------
AEffect *main_plugin(audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main called" << std::endl;

    // don't allow multiple instances
    if (instances == 1) return (0);
    else
    {
        VSTSynth* zynaddsubfx = new VSTSynth (audioMaster);

        if (! zynaddsubfx)
            return 0;

        instances = 1;

        return zynaddsubfx->getAeffect();
    }
}

__attribute__((constructor)) void sharedInit ()
{
    instances = 0;

    config.init ();

    SAMPLE_RATE = config.cfg.SampleRate; //config.cfg.SampleRate;
    SOUND_BUFFER_SIZE = config.cfg.SoundBufferSize; // config.cfg.SoundBufferSize;
    OSCIL_SIZE = config.cfg.OscilSize; // config.cfg.OscilSize;
    config.cfg.UserInterfaceMode = 2;

    denormalkillbuf = new REALTYPE [SOUND_BUFFER_SIZE];
    for (int i = 0; i < SOUND_BUFFER_SIZE; i++)
        denormalkillbuf[i] = (RND - 0.5) * 1e-16;

    OscilGen::tmpsmps = new REALTYPE[OSCIL_SIZE];
    newFFTFREQS (&OscilGen::outoscilFFTfreqs, OSCIL_SIZE / 2);
}

__attribute__((destructor)) void sharedUninit ()
{
    config.save ();

    delete[] denormalkillbuf;
    delete[] OscilGen::tmpsmps;
}

//--------------------------------------------------------------------------------------
// Parts of the VSTSynth class
VSTSynth::VSTSynth (audioMasterCallback audioMaster)
    : AudioEffectX (audioMaster, 1, 0)
{
    setNumInputs (0);
    setNumOutputs (2);
    setUniqueID ((int) "ZASF");
    canProcessReplacing ();
    isSynth (true);
    programsAreChunks (true);

    Midi = new NULLMidiIn();

    vmaster = new Master();
    vmaster->swaplr = config.cfg.SwapStereo;

    hostWindow = None;
    Pexitprogram = 0;
    ui = 0;

    cEffect.flags |= effFlagsHasEditor;

//    pthread_create (&thr, NULL, VSTSynth_GuiThread, this);
}

VSTSynth::~VSTSynth()
{
    Pexitprogram = 1;
    instances = 0;

    usleep (10000);

    delete vmaster;
    delete Midi;
}

//--------------------------------------------------------------------------------------
long VSTSynth::processEvents(VstEvents *events)
{
    for (int i=0;i<events->numEvents;i++)
    {
        if ((events->events[i])->type != kVstMidiType) continue;

        VstMidiEvent *ev= (VstMidiEvent*) events->events[i];

        unsigned char *data= (unsigned char *)ev->midiData;
        int status = data[0] / 16;
        int cmdchan = data[0] & 0x0f;
        int cntl;

        pthread_mutex_lock(&vmaster->mutex);
        switch(status)
        {
            case 0x8:vmaster->NoteOff(cmdchan,data[1]&0x7f);
             break;
            case 0x9:if (data[2]==0) vmaster->NoteOff(cmdchan,data[1]&0x7f);
                  else vmaster->NoteOn(cmdchan,data[1]&0x7f,data[2]&0x7f);
             break;
            case 0xB: cntl=Midi->getcontroller(data[1]&0x7f);
                  vmaster->SetController(cmdchan,cntl,data[2]&0x7f);
             break;
            case 0xE: vmaster->SetController(cmdchan,C_pitchwheel,data[1]+data[2]*(long int) 128-8192);
             break;
        };
        pthread_mutex_unlock(&vmaster->mutex);
    };

    return(1);
};

void VSTSynth::process (float **inputs, float **outputs, long sampleframes)
{
    float *outl=outputs[0];
    float *outr=outputs[1];
    pthread_mutex_lock (&vmaster->mutex);
    vmaster->GetAudioOutSamples (sampleframes, (int) getSampleRate(), outl, outr);
    pthread_mutex_unlock (&vmaster->mutex);
}

void VSTSynth::processReplacing (float **inputs, float **outputs, long sampleframes)
{
    float *outl=outputs[0];
    float *outr=outputs[1];
    pthread_mutex_lock (&vmaster->mutex);
    vmaster->GetAudioOutSamples (sampleframes, (int) getSampleRate(), outl, outr);
    pthread_mutex_unlock (&vmaster->mutex);
}

//--------------------------------------------------------------------------------------
long VSTSynth::getChunk (void** data, bool isPreset)
{
    int size = 0;
    size = vmaster->getalldata ((char **)data);
    return((long)size);
}

long VSTSynth::setChunk (void *data, long size, bool isPreset)
{
    vmaster->putalldata ((char*)data, size);
    return 0;
}

//--------------------------------------------------------------------------------------
long int VSTSynth::canDo(char *txt)
{
    if (strcmp (txt,"receiveVstEvents")!=0) return (1);
    if (strcmp (txt,"receiveVstMidiEvent")!=0) return (1);
    return (-1);
}

bool VSTSynth::getVendorString(char *txt)
{
    strcpy (txt,"Nasca O. Paul");
    return(true);
}

bool VSTSynth::getProductString(char *txt){
    strcpy (txt,"ZynAddSubFX");
    return(true);
};

void VSTSynth::resume(){
    wantEvents();
};

//--------------------------------------------------------------------------------------
// Gui Thread
void* VSTSynth_GuiThread (void *arg)
{
    VSTSynth* vs = (VSTSynth*) arg;

    double waitTime = 0.01;
    while (vs->Pexitprogram == 0)
        Fl::wait (waitTime);

    Fl::flush();
    Fl::wait(0.5);

    pthread_exit (0);
    return 0;
};


//--------------------------------------------------------------------------------------
long VSTSynth::dispatcher (long opCode, long index, long value, void *ptr, float opt)
{
    int result = 0;

    switch (opCode)
    {
    case effSetSampleRate:
        setSampleRate ((int) opt);
        SAMPLE_RATE = (int) opt;
        break;

    case effProcessEvents:
        processEvents ((VstEvents*)ptr);
        result = 1;
        break;

    case effEditIdle:
        break;

    case effEditGetRect:
        if (ui)
        {
            rect.left = 0;
            rect.top = 0;
            switch (config.cfg.UserInterfaceMode)
            {
            case 0:
                rect.right = ui->selectuiwindow->w();
                rect.bottom = ui->selectuiwindow->h();
                break;
            case 1:
                rect.right = ui->masterwindow->w();
                rect.bottom = ui->masterwindow->h();
                break;
            case 2:
                rect.right = ui->simplemasterwindow->w();
                rect.bottom = ui->simplemasterwindow->h();
                break;
            }
        }
        *(ERect**)ptr = &rect;
        result = 1;
        break;

    case effEditOpen:
        hostWindow = (Window) ptr;

        if (display == 0)
            display = (Display*) value;

        if (ui == 0)
        {
            ui = new MasterUI (vmaster, &Pexitprogram);
            ui->showUI ();

            Window embedWindow = None;

            switch (config.cfg.UserInterfaceMode)
            {
            case 0:
                // Fl_X::make_xid (ui->selectuiwindow);
                embedWindow = fl_xid (ui->selectuiwindow);
                break;
            case 1:
                // Fl_X::make_xid (ui->simplemasterwindow);
                embedWindow = fl_xid (ui->masterwindow);
                break;
            case 2:
                // Fl_X::make_xid (ui->simplemasterwindow);
                embedWindow = fl_xid (ui->simplemasterwindow);
                break;
            }

            XReparentWindow (display, embedWindow, hostWindow, 0, 0);

            Pexitprogram = 0;
            pthread_create (&thr, NULL, VSTSynth_GuiThread, this);
        }
        break;

    case effEditClose:
        if (ui)
        {
            delete ui;
            ui = 0;
            
            Pexitprogram = 1;
        }
        break;

    default:
        result = AudioEffect::dispatcher (opCode, index, value, ptr, opt);
    }
    return result;
}

