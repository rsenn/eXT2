<<<<<<< HEAD
/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/

#include <iostream>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "monomakerEditor.cpp"
#include "monomakerEdit.cpp"
#include "monomaker.cpp"


//==============================================================================
extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

extern "C" AEffect *main (audioMasterCallback audioMaster)
{
    MonomakerEdit* const plugin = new MonomakerEdit (audioMaster);

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
=======
/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/

#include <iostream>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "monomakerEditor.cpp"
#include "monomakerEdit.cpp"
#include "monomaker.cpp"


//==============================================================================
extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

extern "C" AEffect *main (audioMasterCallback audioMaster)
{
    MonomakerEdit* const plugin = new MonomakerEdit (audioMaster);

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
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a

