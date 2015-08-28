/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/
#include "BitcrusherEdit.hpp"

#ifndef __BCEditor
#include "BCEditor.hpp"
#endif

#include <string.h>

//-----------------------------------------------------------------------------
BitcrusherEdit::BitcrusherEdit (audioMasterCallback audioMaster)
 : Bitcrusher (audioMaster)
{
	setUniqueID ((int) "Bita");
	editor = 0;
    cEffect.flags |= effFlagsHasEditor; // has editor
}

//-----------------------------------------------------------------------------
BitcrusherEdit::~BitcrusherEdit ()
{
	// the editor gets deleted by the
	// AudioEffect base class
}

//-----------------------------------------------------------------------------
void BitcrusherEdit::setParameter (long index, float value)
{
	Bitcrusher::setParameter (index, value);

	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);
}

//-----------------------------------------------------------------------------------------
long BitcrusherEdit::dispatcher (long opCode, long index, long value, void *ptr, float opt)
{
    int result = 0;

    switch (opCode)
    {
    case effSetSampleRate:
         setSampleRate ((int) opt);
         break;
/*
    case effProcessEvents:
         processEvents ((VstEvents*) ptr);
         result = 1;
         break;
*/
    case effEditClose:
        if (editor)
        {
            editor->close ();
            delete editor;
            editor = 0;
        }
        break;

    case effEditOpen:
        if (display == 0)
            display = (Display*) value;

        if (editor == 0)
            editor = new BCEditor (this);

    default:
        result = AudioEffectX::dispatcher (opCode, index, value, ptr, opt);
    }
    return result;
}
