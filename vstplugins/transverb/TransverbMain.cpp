#include <iostream>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "transverb.hpp"
#include "IIRfilter.cpp"
#include "FIRfilter.cpp"
#include "vstchunk.cpp"
#include "dfxmisc.cpp"
#include "transverbFormalities.cpp"
#include "transverbProcess.cpp"

//-----------------------------------------------------------------------------------------
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

AEffect *main (audioMasterCallback audioMaster)
{
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
