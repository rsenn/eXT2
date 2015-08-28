#ifndef __monomakerEdit
#include "monomakerEdit.hpp"
#endif

#ifndef __monomakerEditor
#include "monomakerEditor.hpp"
#endif


extern bool oome;

//-----------------------------------------------------------------------------
MonomakerEdit::MonomakerEdit(audioMasterCallback audioMaster)
 : Monomaker(audioMaster)
{
	editor = 0;
    cEffect.flags |= effFlagsHasEditor;
}

//-----------------------------------------------------------------------------
MonomakerEdit::~MonomakerEdit()
{
	// the editor gets deleted by the AudioEffect base class
}

//-----------------------------------------------------------------------------
void MonomakerEdit::setParameter(long index, float value)
{
	Monomaker::setParameter(index, value);

	if (editor)
		((AEffGUIEditor*)editor)->setParameter(index, value);
}

//----------------------------------------------------------------------------
long MonomakerEdit::dispatcher (long opCode, long index, long value, void *ptr, float opt)
{
    int result = 0;

    switch (opCode)
    {
    case effSetSampleRate:
         setSampleRate ((int) opt);
         break;
    case effProcessEvents:
         processEvents ((VstEvents*) ptr);
         result = 1;
         break;

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
            editor = new MonomakerEditor (this);

    default:
        result = AudioEffectX::dispatcher (opCode, index, value, ptr, opt);
    }
    return result;
}
