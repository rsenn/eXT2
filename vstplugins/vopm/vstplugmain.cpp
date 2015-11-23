//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4       $Date: 2006/01/12 09:05:31 $
//
// Category     : VST 2.x Classes
// Filename     : vstplugmain.cpp
// Created by   : Steinberg Media Technologies
// Description  : VST Plug-In Main Entry
//
// © 2006, Steinberg Media Technologies, All Rights Reserved
// 2006.3.13 Overwrite sam for Win32_Gcc(Dev C++ )
//-------------------------------------------------------------------------------------------------------

#include <iostream>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "op.cpp"
#include "lfo.cpp"
#include "form.cpp"
#include "disp.cpp"
#include "Slider.cpp"
#include "OPM.cpp"
#include "OPMdrv.cpp"
#include "VOPMproc.cpp"
#include "VOPM.cpp"
#include "VOPMEdit.cpp"


//------------------------------------------------------------------------
/** Must be implemented externally. */
extern AudioEffect* createEffectInstance (audioMasterCallback audioMaster);

//-----------------------------------------------------------------------------------------
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main" << std::endl;

	// Get VST Version
	if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version

	// Create the AudioEffect
	AudioEffect* effect = createEffectInstance (audioMaster);
	if (!effect)
		return 0;

	return effect->getAeffect ();
}


__attribute__((constructor)) void myLoad ()
{
//    std::cout << "myLoad" << std::endl;
}

__attribute__((destructor)) void myUnload ()
{
//    std::cout << "myUnload" << std::endl;
}







