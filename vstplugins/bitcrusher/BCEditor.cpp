<<<<<<< HEAD
/*-----------------------------------------------------------------------------

(C)2003 Marko My��en / CMT
A simple bitcrusher plugin with pre-amp gain, bit-depth and downsampling controls
	v1.00 Initial release with gui
	v1.01 Added 3 distortion types and selector switches for them

Thanks to:
	Ossi Honkanen / Exile for gfx
	Urs Heckermann for coding the radiobutton-group control widget!
	Vesa Norilo for valueble debug-information!-D
-----------------------------------------------------------------------------*/

#include "math.h"

#ifndef __BCEditor
#include "BCEditor.hpp"
#endif

#ifndef __BITCRUSHER_H
#include "Bitcrusher.hpp"
#endif

#include <stdio.h>

//-----------------------------------------------------------------------------
// resource id's
enum {
	// bitmaps
	kBackgroundId = 128,
	kFaderBodyId,
	kFaderHandleId,
	kDistortionSwitchOffId,
	kDistortionSwitchOnId,

	// The position of the first fader
	kFaderX = 15,
	kFaderY = 63,

	// The "offset" space between faders
	kFaderInc = 26,

	// The position of the first button
	kButtonX = 19,
	kButtonY = 44,

	// The "offset" space between buttons
	kButtonInc = 20,

	// The position of the first value display
	kDisplayX = 196,
	kDisplayY = 64,

	// Value display size
	kDisplayXWidth = 28,
	kDisplayHeight = 16
};

#if MOTIF
// resource for MOTIF (format XPM)
#include "images/bmp00128.xpm"
#include "images/bmp00129.xpm"
#include "images/bmp00130.xpm"
#include "images/bmp00131.xpm"
#include "images/bmp00132.xpm"

CResTable xpmResources = {
	{kBackgroundId , bmp00128},
	{kFaderBodyId  , bmp00129},
	{kFaderHandleId, bmp00130},
	{kDistortionSwitchOffId, bmp00131},
	{kDistortionSwitchOnId, bmp00132},
  {0, 0}
};
#endif

// A set of functions that convert slider values to display values

// Converts [0, 1] values to 0-24 integer-values (like bits in the bit-depth parameter)
void float2Bits (float value, char*string)
{
	sprintf (string, "%0.0f", (-23.f*value)+24);	// 1-24 bits in display
}

// Converts [0, 1] values to 0-24 decibels (pre-amp gain parameter)
void float2dB (float value, char*string)
{
	sprintf (string, "%0.1f", 20.*log10(-16.f*value + 17.f));	// Show max. gain of approx. 24dBs
}

// Converts [0, 1] values to integers between 1 and 40. It's the "downsampling" factor
void srateReduction(float value, char*string)
{
	sprintf (string, "%0.0f", -39.f*value + 40.f);
}
//-----------------------------------------------------------------------------
// BCEditor class implementation
//-----------------------------------------------------------------------------
BCEditor::BCEditor (AudioEffect *effect)
 : AEffGUIEditor (effect)
{
	// Init bitmaps
	hFaderBody   = 0;
	hFaderHandle = 0;
	hDistortionSwitchOff = 0;
	hDistortionSwitchOn = 0;

	// Init faders
	bitDepthFader    = 0;
	sampleRateFader = 0;
	gainFader   = 0;

	// Init displays
	bitdepthDisplay = 0;
	samplerateDisplay= 0;
	gainDisplay = 0;

	// Init radiogroup
	distortionButtons = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	hBackground  = new CBitmap (kBackgroundId);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBackground->getWidth ();
	rect.bottom = (short)hBackground->getHeight ();
}

//-----------------------------------------------------------------------------
BCEditor::~BCEditor ()
{
	// free background bitmap
	if (hBackground)
		hBackground->forget ();
	hBackground = 0;
}

//-----------------------------------------------------------------------------
long BCEditor::open (void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open (ptr);

	// load some bitmaps
	if (!hFaderBody)
		hFaderBody   = new CBitmap (kFaderBodyId);

	if (!hFaderHandle)
		hFaderHandle = new CBitmap (kFaderHandleId);

	if (!hDistortionSwitchOff)
		hDistortionSwitchOff = new CBitmap (kDistortionSwitchOffId);

	if (!hDistortionSwitchOn)
		hDistortionSwitchOn = new CBitmap (kDistortionSwitchOnId);


	//--init background frame-----------------------------------------------
	CRect size (0, 0, hBackground->getWidth (), hBackground->getHeight ());
	frame = new CFrame (size, ptr, this);
	frame->setBackground (hBackground);


	//--init the faders------------------------------------------------
	// Set pixel limits fader knobs
	int minPos = kFaderX + 5;
	int maxPos = kFaderX + hFaderBody->getWidth () - hFaderHandle->getWidth () - 1;

	CPoint point (0, 0);
	CPoint offset (1, 2);

	// Create gain fader
	size (kFaderX, kFaderY, kFaderX + hFaderBody->getWidth (), kFaderY + hFaderBody->getHeight ());
	gainFader = new CHorizontalSlider (size, this, kGain, minPos, maxPos, hFaderHandle, hFaderBody, point);
	gainFader->setOffsetHandle (offset);
	gainFader->setValue (effect->getParameter(kGain));
	frame->addView (gainFader);

	// Create bit-depth fader
	size.offset (0, kFaderInc + hFaderBody->getHeight ());
	bitDepthFader = new CHorizontalSlider (size, this, kBitDepth, minPos, maxPos, hFaderHandle, hFaderBody, point);
	bitDepthFader->setOffsetHandle (offset);
	bitDepthFader->setValue (effect->getParameter (kBitDepth));
	frame->addView (bitDepthFader);

	// Create sample-rate fader
	size.offset (0, kFaderInc + hFaderBody->getHeight ());
	sampleRateFader = new CHorizontalSlider (size, this, kSampleRate, minPos, maxPos, hFaderHandle, hFaderBody, point);
	sampleRateFader->setOffsetHandle (offset);
	sampleRateFader->setValue (effect->getParameter (kSampleRate));
	frame->addView (sampleRateFader);

	//--init the display------------------------------------------------

	// Set custom colors for the parameter displays
	CColor bgColor = {208, 216, 183, 0};
	CColor textColor = {119, 124, 106, 0};
	CColor frameColor = {87, 92, 72, 0};

	// Create gain display
	size (kDisplayX, kDisplayY, kDisplayX + kDisplayXWidth, kDisplayY + kDisplayHeight);
	gainDisplay = new CParamDisplay (size, 0, kCenterText);
	gainDisplay->setFont (kNormalFontSmall);
	gainDisplay->setFontColor (textColor);
	gainDisplay->setBackColor (bgColor);
	gainDisplay->setFrameColor (frameColor);
	gainDisplay->setValue (effect->getParameter (kGain));
	gainDisplay->setStringConvert (float2dB);
	frame->addView (gainDisplay);

	// Create bit-depth display
	size.offset(0, kFaderInc + hFaderBody->getHeight ());
	bitdepthDisplay = new CParamDisplay (size, 0, kCenterText);
	bitdepthDisplay->setFont (kNormalFontSmall);
	bitdepthDisplay->setFontColor (textColor);
	bitdepthDisplay->setBackColor (bgColor);
	bitdepthDisplay->setFrameColor (frameColor);
	bitdepthDisplay->setValue (effect->getParameter (kBitDepth));
	bitdepthDisplay->setStringConvert (float2Bits);
	frame->addView (bitdepthDisplay);

	// Create sample-rate display
	size.offset (0, kFaderInc + hFaderBody->getHeight ());
	samplerateDisplay = new CParamDisplay (size, 0, kCenterText);
	samplerateDisplay->setFont (kNormalFontSmall);
	samplerateDisplay->setFontColor (textColor);
	samplerateDisplay->setBackColor (bgColor);
	samplerateDisplay->setFrameColor (frameColor);
	samplerateDisplay->setValue (effect->getParameter (kSampleRate));
	samplerateDisplay->setStringConvert (srateReduction);
	frame->addView (samplerateDisplay);

	//--init radio group------------------------------------------------

	size (kButtonX, kButtonY, kButtonX + kButtonInc*3, kButtonY + hDistortionSwitchOff->getHeight ());
	distortionButtons = new CRadioGroup( size, this, kDistortionType, 3, kButtonInc, 0 );
	// add bitmaps
	distortionButtons->addBitmap ( hDistortionSwitchOff, hDistortionSwitchOn);
	distortionButtons->addBitmap ( hDistortionSwitchOff, hDistortionSwitchOn);
	distortionButtons->addBitmap ( hDistortionSwitchOff, hDistortionSwitchOn);
	// init value
	distortionButtons->setValue (effect->getParameter (kDistortionType));
	// add to frame
	frame->addView (distortionButtons);

	return true;
}

//-----------------------------------------------------------------------------
void BCEditor::close ()
{
    frame->close();
	delete frame;
	frame = 0;

	// free some bitmaps
	if (hFaderBody)
		hFaderBody->forget ();
	hFaderBody = 0;

	if (hFaderHandle)
		hFaderHandle->forget ();
	hFaderHandle = 0;

	if (hDistortionSwitchOff)
		hDistortionSwitchOff->forget ();
	hDistortionSwitchOff = 0;

	if (hDistortionSwitchOn)
		hDistortionSwitchOn->forget ();
	hDistortionSwitchOn = 0;

}

//-----------------------------------------------------------------------------
void BCEditor::setParameter (long index, float value)
{
	if (!frame)
		return;

	// called from BitcrusherEdit
	switch (index)
	{
	case kDistortionType:
		if (distortionButtons)
			distortionButtons->setValue (effect->getParameter (index));
		break;

	case kGain:
		if (gainFader)
			gainFader->setValue (effect->getParameter (index));
		if (gainDisplay)
			gainDisplay->setValue (effect->getParameter (index));
		break;

	case kBitDepth:
		if (bitDepthFader)
			bitDepthFader->setValue (effect->getParameter (index));
		if (bitdepthDisplay)
			bitdepthDisplay->setValue (effect->getParameter (index));
		break;

	case kSampleRate:
		if (sampleRateFader)
			sampleRateFader->setValue (effect->getParameter (index));
		if (samplerateDisplay)
			samplerateDisplay->setValue (effect->getParameter(index));
		break;
	}
	postUpdate ();
}

//-----------------------------------------------------------------------------
void BCEditor::valueChanged (CDrawContext* context, CControl* control)
{
	long tag = control->getTag ();
	switch (tag)
	{
	case kDistortionType:
	case kBitDepth:
	case kSampleRate:
	case kGain:
		effect->setParameterAutomated (tag, control->getValue ());
		control->update (context);
		break;
	}
}
=======
/*-----------------------------------------------------------------------------

(C)2003 Marko My��en / CMT
A simple bitcrusher plugin with pre-amp gain, bit-depth and downsampling controls
	v1.00 Initial release with gui
	v1.01 Added 3 distortion types and selector switches for them

Thanks to:
	Ossi Honkanen / Exile for gfx
	Urs Heckermann for coding the radiobutton-group control widget!
	Vesa Norilo for valueble debug-information!-D
-----------------------------------------------------------------------------*/

#include "math.h"

#ifndef __BCEditor
#include "BCEditor.hpp"
#endif

#ifndef __BITCRUSHER_H
#include "Bitcrusher.hpp"
#endif

#include <stdio.h>

//-----------------------------------------------------------------------------
// resource id's
enum {
	// bitmaps
	kBackgroundId = 128,
	kFaderBodyId,
	kFaderHandleId,
	kDistortionSwitchOffId,
	kDistortionSwitchOnId,

	// The position of the first fader
	kFaderX = 15,
	kFaderY = 63,

	// The "offset" space between faders
	kFaderInc = 26,

	// The position of the first button
	kButtonX = 19,
	kButtonY = 44,

	// The "offset" space between buttons
	kButtonInc = 20,

	// The position of the first value display
	kDisplayX = 196,
	kDisplayY = 64,

	// Value display size
	kDisplayXWidth = 28,
	kDisplayHeight = 16
};

#if MOTIF
// resource for MOTIF (format XPM)
#include "images/bmp00128.xpm"
#include "images/bmp00129.xpm"
#include "images/bmp00130.xpm"
#include "images/bmp00131.xpm"
#include "images/bmp00132.xpm"

CResTable xpmResources = {
	{kBackgroundId , bmp00128},
	{kFaderBodyId  , bmp00129},
	{kFaderHandleId, bmp00130},
	{kDistortionSwitchOffId, bmp00131},
	{kDistortionSwitchOnId, bmp00132},
  {0, 0}
};
#endif

// A set of functions that convert slider values to display values

// Converts [0, 1] values to 0-24 integer-values (like bits in the bit-depth parameter)
void float2Bits (float value, char*string)
{
	sprintf (string, "%0.0f", (-23.f*value)+24);	// 1-24 bits in display
}

// Converts [0, 1] values to 0-24 decibels (pre-amp gain parameter)
void float2dB (float value, char*string)
{
	sprintf (string, "%0.1f", 20.*log10(-16.f*value + 17.f));	// Show max. gain of approx. 24dBs
}

// Converts [0, 1] values to integers between 1 and 40. It's the "downsampling" factor
void srateReduction(float value, char*string)
{
	sprintf (string, "%0.0f", -39.f*value + 40.f);
}
//-----------------------------------------------------------------------------
// BCEditor class implementation
//-----------------------------------------------------------------------------
BCEditor::BCEditor (AudioEffect *effect)
 : AEffGUIEditor (effect)
{
	// Init bitmaps
	hFaderBody   = 0;
	hFaderHandle = 0;
	hDistortionSwitchOff = 0;
	hDistortionSwitchOn = 0;

	// Init faders
	bitDepthFader    = 0;
	sampleRateFader = 0;
	gainFader   = 0;

	// Init displays
	bitdepthDisplay = 0;
	samplerateDisplay= 0;
	gainDisplay = 0;

	// Init radiogroup
	distortionButtons = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	hBackground  = new CBitmap (kBackgroundId);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBackground->getWidth ();
	rect.bottom = (short)hBackground->getHeight ();
}

//-----------------------------------------------------------------------------
BCEditor::~BCEditor ()
{
	// free background bitmap
	if (hBackground)
		hBackground->forget ();
	hBackground = 0;
}

//-----------------------------------------------------------------------------
long BCEditor::open (void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open (ptr);

	// load some bitmaps
	if (!hFaderBody)
		hFaderBody   = new CBitmap (kFaderBodyId);

	if (!hFaderHandle)
		hFaderHandle = new CBitmap (kFaderHandleId);

	if (!hDistortionSwitchOff)
		hDistortionSwitchOff = new CBitmap (kDistortionSwitchOffId);

	if (!hDistortionSwitchOn)
		hDistortionSwitchOn = new CBitmap (kDistortionSwitchOnId);


	//--init background frame-----------------------------------------------
	CRect size (0, 0, hBackground->getWidth (), hBackground->getHeight ());
	frame = new CFrame (size, ptr, this);
	frame->setBackground (hBackground);


	//--init the faders------------------------------------------------
	// Set pixel limits fader knobs
	int minPos = kFaderX + 5;
	int maxPos = kFaderX + hFaderBody->getWidth () - hFaderHandle->getWidth () - 1;

	CPoint point (0, 0);
	CPoint offset (1, 2);

	// Create gain fader
	size (kFaderX, kFaderY, kFaderX + hFaderBody->getWidth (), kFaderY + hFaderBody->getHeight ());
	gainFader = new CHorizontalSlider (size, this, kGain, minPos, maxPos, hFaderHandle, hFaderBody, point);
	gainFader->setOffsetHandle (offset);
	gainFader->setValue (effect->getParameter(kGain));
	frame->addView (gainFader);

	// Create bit-depth fader
	size.offset (0, kFaderInc + hFaderBody->getHeight ());
	bitDepthFader = new CHorizontalSlider (size, this, kBitDepth, minPos, maxPos, hFaderHandle, hFaderBody, point);
	bitDepthFader->setOffsetHandle (offset);
	bitDepthFader->setValue (effect->getParameter (kBitDepth));
	frame->addView (bitDepthFader);

	// Create sample-rate fader
	size.offset (0, kFaderInc + hFaderBody->getHeight ());
	sampleRateFader = new CHorizontalSlider (size, this, kSampleRate, minPos, maxPos, hFaderHandle, hFaderBody, point);
	sampleRateFader->setOffsetHandle (offset);
	sampleRateFader->setValue (effect->getParameter (kSampleRate));
	frame->addView (sampleRateFader);

	//--init the display------------------------------------------------

	// Set custom colors for the parameter displays
	CColor bgColor = {208, 216, 183, 0};
	CColor textColor = {119, 124, 106, 0};
	CColor frameColor = {87, 92, 72, 0};

	// Create gain display
	size (kDisplayX, kDisplayY, kDisplayX + kDisplayXWidth, kDisplayY + kDisplayHeight);
	gainDisplay = new CParamDisplay (size, 0, kCenterText);
	gainDisplay->setFont (kNormalFontSmall);
	gainDisplay->setFontColor (textColor);
	gainDisplay->setBackColor (bgColor);
	gainDisplay->setFrameColor (frameColor);
	gainDisplay->setValue (effect->getParameter (kGain));
	gainDisplay->setStringConvert (float2dB);
	frame->addView (gainDisplay);

	// Create bit-depth display
	size.offset(0, kFaderInc + hFaderBody->getHeight ());
	bitdepthDisplay = new CParamDisplay (size, 0, kCenterText);
	bitdepthDisplay->setFont (kNormalFontSmall);
	bitdepthDisplay->setFontColor (textColor);
	bitdepthDisplay->setBackColor (bgColor);
	bitdepthDisplay->setFrameColor (frameColor);
	bitdepthDisplay->setValue (effect->getParameter (kBitDepth));
	bitdepthDisplay->setStringConvert (float2Bits);
	frame->addView (bitdepthDisplay);

	// Create sample-rate display
	size.offset (0, kFaderInc + hFaderBody->getHeight ());
	samplerateDisplay = new CParamDisplay (size, 0, kCenterText);
	samplerateDisplay->setFont (kNormalFontSmall);
	samplerateDisplay->setFontColor (textColor);
	samplerateDisplay->setBackColor (bgColor);
	samplerateDisplay->setFrameColor (frameColor);
	samplerateDisplay->setValue (effect->getParameter (kSampleRate));
	samplerateDisplay->setStringConvert (srateReduction);
	frame->addView (samplerateDisplay);

	//--init radio group------------------------------------------------

	size (kButtonX, kButtonY, kButtonX + kButtonInc*3, kButtonY + hDistortionSwitchOff->getHeight ());
	distortionButtons = new CRadioGroup( size, this, kDistortionType, 3, kButtonInc, 0 );
	// add bitmaps
	distortionButtons->addBitmap ( hDistortionSwitchOff, hDistortionSwitchOn);
	distortionButtons->addBitmap ( hDistortionSwitchOff, hDistortionSwitchOn);
	distortionButtons->addBitmap ( hDistortionSwitchOff, hDistortionSwitchOn);
	// init value
	distortionButtons->setValue (effect->getParameter (kDistortionType));
	// add to frame
	frame->addView (distortionButtons);

	return true;
}

//-----------------------------------------------------------------------------
void BCEditor::close ()
{
    frame->close();
	delete frame;
	frame = 0;

	// free some bitmaps
	if (hFaderBody)
		hFaderBody->forget ();
	hFaderBody = 0;

	if (hFaderHandle)
		hFaderHandle->forget ();
	hFaderHandle = 0;

	if (hDistortionSwitchOff)
		hDistortionSwitchOff->forget ();
	hDistortionSwitchOff = 0;

	if (hDistortionSwitchOn)
		hDistortionSwitchOn->forget ();
	hDistortionSwitchOn = 0;

}

//-----------------------------------------------------------------------------
void BCEditor::setParameter (long index, float value)
{
	if (!frame)
		return;

	// called from BitcrusherEdit
	switch (index)
	{
	case kDistortionType:
		if (distortionButtons)
			distortionButtons->setValue (effect->getParameter (index));
		break;

	case kGain:
		if (gainFader)
			gainFader->setValue (effect->getParameter (index));
		if (gainDisplay)
			gainDisplay->setValue (effect->getParameter (index));
		break;

	case kBitDepth:
		if (bitDepthFader)
			bitDepthFader->setValue (effect->getParameter (index));
		if (bitdepthDisplay)
			bitdepthDisplay->setValue (effect->getParameter (index));
		break;

	case kSampleRate:
		if (sampleRateFader)
			sampleRateFader->setValue (effect->getParameter (index));
		if (samplerateDisplay)
			samplerateDisplay->setValue (effect->getParameter(index));
		break;
	}
	postUpdate ();
}

//-----------------------------------------------------------------------------
void BCEditor::valueChanged (CDrawContext* context, CControl* control)
{
	long tag = control->getTag ();
	switch (tag)
	{
	case kDistortionType:
	case kBitDepth:
	case kSampleRate:
	case kGain:
		effect->setParameterAutomated (tag, control->getValue ());
		control->update (context);
		break;
	}
}
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
