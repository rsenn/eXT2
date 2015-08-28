#include <iostream>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "CycleShifter.cpp"
#include "CycleShifterEditor.cpp"

//==============================================================================
extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

extern "C" AEffect *main (audioMasterCallback audioMaster)
{
//    std::cout << "main" << std::endl;

    CycleShifter* const plugin = new CycleShifter (audioMaster);

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
