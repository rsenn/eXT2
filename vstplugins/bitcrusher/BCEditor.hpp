/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/
#ifndef __BCEditor
#define __BCEditor


// include VSTGUI
#ifndef __vstgui__
#include "../vstgui/vstgui.h"
#endif

// Include Urs Heckmanns radio group control class
#include "c_radio_group.hpp"

//-----------------------------------------------------------------------------
class BCEditor : public AEffGUIEditor, public CControlListener
{
public:
	BCEditor (AudioEffect *effect);
	virtual ~BCEditor ();

protected:
	virtual long open (void *ptr);
	virtual void close ();

	virtual void setParameter (long index, float value);
	virtual void valueChanged (CDrawContext* context, CControl* control);

private:
	// Sliders
	CHorizontalSlider *gainFader;
	CHorizontalSlider *bitDepthFader;
	CHorizontalSlider *sampleRateFader;

	// Parameter displays
	CParamDisplay *gainDisplay;
	CParamDisplay *bitdepthDisplay;
	CParamDisplay *samplerateDisplay;

	// Radiogroup
	CRadioGroup *distortionButtons;

	// Bitmaps
	CBitmap *hBackground;
	CBitmap *hFaderBody;
	CBitmap *hFaderHandle;
	CBitmap *hDistortionSwitchOff;
	CBitmap *hDistortionSwitchOn;
};

#endif
