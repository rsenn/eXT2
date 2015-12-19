<<<<<<< HEAD
/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/

#include <iostream>

#include "AudioEffect.cpp"
#include "audioeffectx.h"
#include "audioeffectx.cpp"
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "Bitcrusher.cpp"
#include "BitcrusherEdit.cpp"
#include "BCEditor.cpp"
#include "c_radio_group.cpp"

//==============================================================================
extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

extern "C" AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main" << std::endl;

    BitcrusherEdit* const plugin = new BitcrusherEdit (audioMaster);

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

#include "AudioEffect.cpp"
#include "audioeffectx.h"
#include "audioeffectx.cpp"
#include "../vstgui/vstgui.cpp"
#include "../vstgui/vstcontrols.cpp"

#include "Bitcrusher.cpp"
#include "BitcrusherEdit.cpp"
#include "BCEditor.cpp"
#include "c_radio_group.cpp"

//==============================================================================
extern "C" AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

extern "C" AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main" << std::endl;

    BitcrusherEdit* const plugin = new BitcrusherEdit (audioMaster);

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
