#include <iostream>

<<<<<<< HEAD
#include <AudioEffect.cpp>
#include <audioeffectx.h>
=======
#include <AudioEffect.cpp>
#include <audioeffectx.h>
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
#include <audioeffectx.cpp>

#include "transverb.hpp"
#include "IIRfilter.cpp"
#include "FIRfilter.cpp"
#include "vstchunk.cpp"
#include "dfxmisc.cpp"
#include "transverbFormalities.cpp"
#include "transverbProcess.cpp"

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

	Transverb* effect = new Transverb (audioMaster);

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
