//----------------------------------------------------------------------------
/*
    ladspa2vst - ladspa 2 vst plugins bridge class

    Copyright (C) 2007 kRAkEn/gORe

    This is experimental software. Is based around ladspa plugins
    specification and steinberg vst specifications.

    To compile your ladspa plugins under a vst environment you will
    need to download a ladspa plugin, the complete vst sdk from steinberg.
    then define you LADSPA_INIT function (typically _init) and
    LADSPA_FINI function (typically _fini) and what is the global
    descriptor for the plugin (usually g_psDescriptor). then include
    you ladspa .h/.c file and compile. This would produce a shared
    library loadable from any linux native vst hosts.

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
//----------------------------------------------------------------------------

#include "ladspa2vst_def.h"


//-----------------------------------------------------------------------------------------
static const LADSPA_Descriptor* psDescriptor = 0;

//-----------------------------------------------------------------------------------------
int countLADSPAPluginParameters (const LADSPA_Descriptor* psDescriptor)
{
   // @XXX - some ports are only output control, so useless as parameters

   int numParameters = 0;
   for (unsigned int i = 0; i < psDescriptor->PortCount; i++)
   {
        if ((psDescriptor->PortDescriptors [i] & LADSPA_PORT_INPUT)
            && (psDescriptor->PortDescriptors [i] & LADSPA_PORT_CONTROL))
        {
            numParameters++;
        }
   }
   return numParameters;
}


//-----------------------------------------------------------------------------------------
class LadspaParameter
{
public:

    LadspaParameter (int portIndex_)
      : portIndex (portIndex_),
        minValue (0.0f),
        maxValue (1.0f),
        currentValue (0.0f)
    {}

    int portIndex;
    float minValue;
    float maxValue;
    float currentValue;
};


//-----------------------------------------------------------------------------------------
class LadspaHost : public AudioEffectX
{
public:
    LadspaHost (audioMasterCallback audioMaster, int params);
    ~LadspaHost ();

    virtual void    process (float **inputs, float **outputs, long sampleFrames);
    virtual void    processReplacing (float **inputs, float **outputs, long sampleFrames);
    void            processAudio (float **inputs, float **outputs, long sampleFrames, bool replace);

    virtual void    resume ();
    virtual void    suspend ();

    virtual void    setParameter (long index, float value);
    virtual float   getParameter (long index);
    virtual void    getParameterName (long index, char *text);
    virtual void    getParameterDisplay (long index, char *text);

    virtual void    setSampleRate (float sampleRate);

    long dispatcher (long opCode, long index, long value, void *ptr, float opt);

protected:

    LADSPA_Handle plugin;

    int ins [32];
    int outs [32];
    int pars [32];

    int numInputs;
    int numOutputs;
    int numParams;

    float* parameters;
    float* normalized;

    float samplingRate;
};

//-----------------------------------------------------------------------------
LadspaHost::LadspaHost (audioMasterCallback audioMaster, int params)
    : AudioEffectX (audioMaster, 1, params),
      plugin (0)
{
    samplingRate = 44100.0;

    plugin = psDescriptor->instantiate (psDescriptor, (unsigned int) samplingRate);

    numInputs = 0, numOutputs = 0, numParams = 0;
    for (unsigned int i = 0; i < psDescriptor->PortCount; i++)
    {
        if (psDescriptor->PortDescriptors [i] & LADSPA_PORT_CONTROL) pars [numParams++] = i;
        if (psDescriptor->PortDescriptors [i] & LADSPA_PORT_AUDIO)
        {
            if (psDescriptor->PortDescriptors [i] & LADSPA_PORT_INPUT)    ins [numInputs++] = i;
            if (psDescriptor->PortDescriptors [i] & LADSPA_PORT_OUTPUT)   outs [numOutputs++] = i;
        }
    }

    setUniqueID ((int) psDescriptor->UniqueID);
    setNumInputs (numInputs);   // stereo in
    setNumOutputs (numOutputs); // stereo out
    if (psDescriptor->run)
        canProcessReplacing();  // supports both accumulating and replacing output
    // canMono();               // makes sense to feed both inputs with the same signal
    // hasVu();

    parameters = new float [numParams];
    normalized = new float [numParams];
    memset (parameters, 0, numParams * sizeof (float));
    memset (normalized, 0, numParams * sizeof (float));

    for (int i = 0; i < numParams; i++)
        psDescriptor->connect_port (plugin, pars [i], &normalized[i]);

    for (int i = 0; i < numParams; i++)
    {
        const LADSPA_PortRangeHint* hint = & psDescriptor->PortRangeHints [pars [i]];

        if (LADSPA_IS_HINT_HAS_DEFAULT (hint->HintDescriptor))
        {
            if (LADSPA_IS_HINT_DEFAULT_0 (hint->HintDescriptor))
            {
                normalized [i] = 0.0f;
            }
            if (LADSPA_IS_HINT_DEFAULT_1 (hint->HintDescriptor))
            {
                normalized [i] = 1.0f;
            }
            if (LADSPA_IS_HINT_DEFAULT_100 (hint->HintDescriptor))
            {
                normalized [i] = 100.0f;
            }
            if (LADSPA_IS_HINT_DEFAULT_440 (hint->HintDescriptor))
            {
                normalized [i] = 440.0f;
            }
            if (LADSPA_IS_HINT_DEFAULT_MINIMUM (hint->HintDescriptor))
            {
                normalized [i] = hint->LowerBound;
                parameters [i] = 0.0f;
            }
            if (LADSPA_IS_HINT_DEFAULT_LOW (hint->HintDescriptor))
            {
                normalized [i] = hint->LowerBound * 0.75 + hint->UpperBound * 0.25;
                parameters [i] = 0.25f;
            }
            if (LADSPA_IS_HINT_DEFAULT_MIDDLE (hint->HintDescriptor))
            {
                normalized [i] = (hint->UpperBound - hint->LowerBound) * 0.5f;
                parameters [i] = 0.5f;
            }
            if (LADSPA_IS_HINT_DEFAULT_HIGH (hint->HintDescriptor))
            {
                normalized [i] = hint->LowerBound * 0.25 + hint->UpperBound * 0.75;
                parameters [i] = 0.75f;
            }
            if (LADSPA_IS_HINT_DEFAULT_MAXIMUM (hint->HintDescriptor))
            {
                normalized [i] = hint->UpperBound;
                parameters [i] = 1.0f;
            }
        }
        else
        {
            normalized [i] = 0.0f;
        }
    }
}

//-----------------------------------------------------------------------------------------
LadspaHost::~LadspaHost()
{
    if (psDescriptor->cleanup)
        psDescriptor->cleanup (plugin);

    delete[] parameters;
    delete[] normalized;
}

//-----------------------------------------------------------------------------------------
void LadspaHost::setParameter (long index, float value)
{
    const LADSPA_PortRangeHint* hint = & psDescriptor->PortRangeHints [pars [index]];

    // @TODO - Handle log scale of parameters
    // @TODO - Handle samplerate changes
    // @TODO - Handle better lower/upper bound. this is ok for most cases
    //         but in some others it don't

    if (LADSPA_IS_HINT_TOGGLED (hint->HintDescriptor))
    {
        if (value < 0.5f)   normalized [index] = 0.0f;
        else                normalized [index] = 1.0f;
    }
    else if (LADSPA_IS_HINT_BOUNDED_BELOW (hint->HintDescriptor)
             && LADSPA_IS_HINT_BOUNDED_ABOVE (hint->HintDescriptor))
    {
        normalized [index] = hint->LowerBound + (hint->UpperBound - hint->LowerBound) * value;
    }
    else if (LADSPA_IS_HINT_BOUNDED_BELOW (hint->HintDescriptor))
    {
        normalized [index] = value;
    }
    else if (LADSPA_IS_HINT_BOUNDED_ABOVE (hint->HintDescriptor))
    {
        normalized [index] = value * hint->UpperBound;
    }

    if (LADSPA_IS_HINT_INTEGER (hint->HintDescriptor))
        normalized [index] = (float) ((int) normalized [index]);

    parameters [index] = value;
}

//-----------------------------------------------------------------------------------------
float LadspaHost::getParameter (long index)
{
    return parameters [index];
}

//-----------------------------------------------------------------------------------------
void LadspaHost::getParameterName (long index, char *text)
{
    strncpy (text, psDescriptor->PortNames [pars [index]], 24);
}

//-----------------------------------------------------------------------------------------
void LadspaHost::getParameterDisplay (long index, char *text)
{
    const LADSPA_PortRangeHint* hint = & psDescriptor->PortRangeHints [pars [index]];

    if (LADSPA_IS_HINT_INTEGER (hint->HintDescriptor))
        sprintf (text, "%d", (int) normalized [index]);
    else
        sprintf (text, "%.2f", normalized [index]);
}

//-----------------------------------------------------------------------------------------
void LadspaHost::setSampleRate (float sampleRate)
{
    samplingRate = sampleRate;
}

//-----------------------------------------------------------------------------------------
void LadspaHost::resume ()
{
    if (psDescriptor->activate)
        psDescriptor->activate (plugin);
    wantEvents (false);
}

//-----------------------------------------------------------------------------------------
void LadspaHost::suspend ()
{
    if (psDescriptor->deactivate)
        psDescriptor->deactivate (plugin);
    wantEvents (false);
}

//-----------------------------------------------------------------------------------------
void LadspaHost::process (float **inputs, float **outputs, long sampleFrames)
{
    processAudio (inputs, outputs, sampleFrames, false);
}

//-----------------------------------------------------------------------------------------
void LadspaHost::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
    processAudio (inputs, outputs, sampleFrames, true);
}

//-----------------------------------------------------------------------------------------
void LadspaHost::processAudio (float **inputs, float **outputs, long sampleFrames, bool replace)
{
    for (int i = 0; i < numInputs; i++)
        psDescriptor->connect_port (plugin, ins [i], inputs[i]);
    for (int i = 0; i < numOutputs; i++)
        psDescriptor->connect_port (plugin, outs [i], outputs[i]);

    if (replace && psDescriptor->run)
        psDescriptor->run (plugin, sampleFrames);
    else if (! replace && psDescriptor->run_adding)
        psDescriptor->run_adding (plugin, sampleFrames);
}

//-----------------------------------------------------------------------------------------
long LadspaHost::dispatcher (long opCode, long index, long value, void *ptr, float opt)
{
    int result = 0;

    switch (opCode)
    {
        case effSetSampleRate:
            setSampleRate ((float)((int) opt));
            break;

        default:
            result = AudioEffect::dispatcher (opCode, index, value, ptr, opt);
    }

    return result;
}

//-----------------------------------------------------------------------------------------
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");

AEffect *main_plugin (audioMasterCallback audioMaster)
{
    if (psDescriptor)
    {
        LadspaHost* effect = new LadspaHost (audioMaster,
                                             countLADSPAPluginParameters (psDescriptor));
        if (effect == 0)
            return 0;

        return effect->getAeffect ();
    }

    return 0;
}

__attribute__((constructor)) void myLoad ()
{
    LADSPA_INIT ();

    psDescriptor = LADSPA_DESCRIPTOR;
}

__attribute__((destructor)) void myUnload ()
{
    LADSPA_FINI ();
}

