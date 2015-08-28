/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/

#include <iostream>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "flp.cpp"
#include "flpproc.cpp"
#include "flp_frame.cpp"
#include "voice_classes.cpp"
#include "Controls.cpp"
#include "Editor.cpp"
#include "fx.cpp"

//==============================================================================
extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

extern "C" AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main" << std::endl;

    flp* const plugin = new flp (audioMaster);

    if (plugin)
        return plugin->getAeffect();

    return 0;
}

__attribute__((constructor)) void myPluginInit() // this is called when the library is unoaded
{
//    std::cout << "myPluginInit" << std::endl;
}

__attribute__((destructor)) void myPluginFini() // this is called when the library is unoaded
{
//    std::cout << "myPluginFini" << std::endl;
}
