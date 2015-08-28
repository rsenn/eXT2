#ifndef __SkidderEdit
#include "SkidderEdit.hpp"
#endif

#ifndef __SkidderEditor
#include "SkidderEditor.hpp"
#endif


//-----------------------------------------------------------------------------
SkidderEdit::SkidderEdit(audioMasterCallback audioMaster)
 : Skidder(audioMaster)
{
	editor = 0;
    cEffect.flags |= effFlagsHasEditor;	
}

//-----------------------------------------------------------------------------
SkidderEdit::~SkidderEdit()
{
	// the editor gets deleted by the AudioEffect base class
}

//-----------------------------------------------------------------------------
void SkidderEdit::setParameter(long index, float value)
{
	Skidder::setParameter(index, value);

	if (editor)
		((AEffGUIEditor*)editor)->setParameter(index, value);
}

//----------------------------------------------------------------------------
long SkidderEdit::dispatcher (long opCode, long index, long value, void *ptr, float opt)
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
            editor = new SkidderEditor (this);

    default:
        result = AudioEffectX::dispatcher (opCode, index, value, ptr, opt);
    }
    return result;
}
