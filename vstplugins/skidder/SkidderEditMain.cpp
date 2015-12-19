#include <iostream>

<<<<<<< HEAD
#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
=======
#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
#include "../vstgui/vstcontrols.cpp"

#include "vstchunk.cpp"
#include "dfxmisc.cpp"
#include "dfxgui.cpp"
#include "dfxguiMultiControls.cpp"
#include "TempoRateTable.cpp"
#include "mskidderMidi.cpp"
#include "skidderFormalities.cpp"
#include "skidderProcess.cpp"
#include "SkidderEditor.cpp"
#include "SkidderEdit.cpp"

#ifndef __SkidderEdit
#include "SkidderEdit.hpp"
#endif

<<<<<<< HEAD
//-----------------------------------------------------------------------------------------
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

AEffect *main (audioMasterCallback audioMaster)
{
=======
//-----------------------------------------------------------------------------------------
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

AEffect *main (audioMasterCallback audioMaster)
{
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
//    std::cout << "main" << std::endl;

	SkidderEdit* effect = new SkidderEdit (audioMaster);

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
