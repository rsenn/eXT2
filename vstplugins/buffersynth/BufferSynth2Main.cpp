//	BufferSynth2Main.cpp
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include <iostream>
#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "EndianSwapFunctions.cpp"
#include "ParameterFilter.cpp"
#include "RandomFlamingLipsName.cpp"
#include "bs2wfl.cpp"
#include "wavefileloader.cpp"

#include "BufferSynth2.cpp"
#include "bs2voice.cpp"
#include "bs2notemaster.cpp"
#include "bs2SetPatches.cpp"
#include "bs2PresetData.cpp"


//-----------------------------------------------------------------------------------------
AEffect *main_plugin (audioMasterCallback audioMaster) asm("main");
#define main main_plugin

AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main" << std::endl;

	BufferSynth2* effect = new BufferSynth2 (audioMaster);

	if (!effect)
		return 0;

	return effect->getAeffect ();
}

//-----------------------------------------------------------------------------------------
__attribute__((constructor)) void myLoad ()
{
//    std::cout << "myLoad" << std::endl;
}

__attribute__((destructor)) void myUnload ()
{
//    std::cout << "myUnload" << std::endl;
}

