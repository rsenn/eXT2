#ifndef __monomakerEditor
#include "monomakerEditor.hpp"
#endif

#ifndef __monomaker
#include "monomaker.hpp"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


//-----------------------------------------------------------------------------
enum {
	// resource IDs
	kBackgroundID = 128,
	kMonomergeFaderSlideID,
	kPanFaderSlideID,
	kFaderHandleID,
	kMonomergeMovieID,
	kPanMovieID,
	kDestroyFXlinkID,

	// positions
	kFaderX = 15,
	kFaderY = 81,
	kFaderInc = 61,

	kDisplayX = 252,
	kDisplayY = 77,
	kDisplayWidth = 83,
	kDisplayHeight = 12,

	kMonomergeMovieX = 14,
	kMonomergeMovieY = 28,
	kPanMovieX = 15,
	kPanMovieY = 116,

	kDestroyFXlinkX = 270,
	kDestroyFXlinkY = 3
};

#if MOTIF
// resource for MOTIF (format XPM)
#include "images/monomaker128.xpm"
#include "images/monomaker129.xpm"
#include "images/monomaker130.xpm"
#include "images/monomaker131.xpm"
#include "images/monomaker132.xpm"
#include "images/monomaker133.xpm"
#include "images/monomaker134.xpm"

CResTable xpmResources = {
	{kBackgroundID,             monomaker128_xpm},
	{kMonomergeFaderSlideID,    monomaker129_xpm},
	{kPanFaderSlideID,          monomaker130_xpm},
	{kFaderHandleID,            monomaker131_xpm},
	{kMonomergeMovieID,         monomaker132_xpm},
	{kPanMovieID,               monomaker133_xpm},
	{kDestroyFXlinkID,          monomaker134_xpm},
	{0, 0}
};
#endif



//-----------------------------------------------------------------------------
// parameter value string display conversion functions

void monomergeDisplayConvert(float value, char *string);
void monomergeDisplayConvert(float value, char *string)
{
	sprintf(string, " %.1f %%", value*100.0f);
}

void panDisplayConvert(float value, char *string);
void panDisplayConvert(float value, char *string)
{
	if (value >= 0.5004f)
		sprintf(string, " +%.3f", (value*2.0f)-1.0f);
	else
		sprintf(string, " %.3f", (value*2.0f)-1.0f);
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MonomakerEditor::MonomakerEditor(AudioEffect *effect)
 : AEffGUIEditor(effect) 
{
	frame = 0;

	// initialize the graphics pointers
	gBackground = 0;
	gMonomergeFaderSlide = 0;
	gPanFaderSlide = 0;
	gFaderHandle = 0;
	gMonomergeMovie = 0;
	gPanMovie = 0;
	gDestroyFXlink = 0;

	// initialize the controls pointers
	monomergeFader = 0;
	panFader = 0;
	monomergeMovie = 0;
	panMovie = 0;
	DestroyFXlink = 0;

	// initialize the value display box pointers
	monomergeDisplay = 0;
	panDisplay = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	gBackground = new CBitmap (kBackgroundID);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)gBackground->getWidth();
	rect.bottom = (short)gBackground->getHeight();
}

//-----------------------------------------------------------------------------
MonomakerEditor::~MonomakerEditor()
{
	// free background bitmap
	if (gBackground)
		gBackground->forget();
	gBackground = 0;
}

//-----------------------------------------------------------------------------
long MonomakerEditor::open(void *ptr)
{
  CPoint displayOffset;	// for positioning the background graphic behind display boxes


	// !!! always call this !!!
	AEffGUIEditor::open(ptr);

	// load some graphics
	if (!gMonomergeFaderSlide)
		gMonomergeFaderSlide = new CBitmap (kMonomergeFaderSlideID);
	if (!gPanFaderSlide)
		gPanFaderSlide = new CBitmap (kPanFaderSlideID);
	if (!gFaderHandle)
		gFaderHandle = new CBitmap (kFaderHandleID);
	if (!gMonomergeMovie)
		gMonomergeMovie = new CBitmap (kMonomergeMovieID);
	if (!gPanMovie)
		gPanMovie = new CBitmap (kPanMovieID);
	if (!gDestroyFXlink)
		gDestroyFXlink = new CBitmap (kDestroyFXlinkID);


	//--initialize the background frame--------------------------------------
	CRect size (0, 0, gBackground->getWidth(), gBackground->getHeight());
	frame = new CFrame (size, ptr, this);
	frame->setBackground(gBackground);


	//--initialize the horizontal faders-------------------------------------
	int minPos = kFaderX;
	int maxPos = kFaderX + gMonomergeFaderSlide->getWidth() - gFaderHandle->getWidth();
	int numMovieFrames;
	CPoint point (0, 0);

	// monomerge
	// size stores left, top, right, & bottom positions
	size (kFaderX, kFaderY, kFaderX + gMonomergeFaderSlide->getWidth(), kFaderY + gMonomergeFaderSlide->getHeight());
	monomergeFader = new CHorizontalSlider (size, this, kMonomerge, minPos, maxPos, gFaderHandle, gMonomergeFaderSlide, point, kLeft);
	monomergeFader->setValue(effect->getParameter(kMonomerge));
	monomergeFader->setDefaultValue(1.0f);
	frame->addView(monomergeFader);

	// pan
	size.offset (0, kFaderInc);
	panFader = new CHorizontalSlider (size, this, kPan, minPos, maxPos, gFaderHandle, gPanFaderSlide, point, kLeft);
	panFader->setValue(effect->getParameter(kPan));
	panFader->setDefaultValue(0.5f);
	frame->addView(panFader);

	// monomerge movie
	numMovieFrames = 19;
	size (kMonomergeMovieX, kMonomergeMovieY, kMonomergeMovieX + gMonomergeMovie->getWidth(), kMonomergeMovieY + (gMonomergeMovie->getHeight()/numMovieFrames));
	monomergeMovie = new CMovieBitmap (size, this, kMonomerge, numMovieFrames, gMonomergeMovie->getHeight()/numMovieFrames, gMonomergeMovie, point);
	monomergeMovie->setValue(effect->getParameter(kMonomerge));
	frame->addView(monomergeMovie);

	// pan movie
	numMovieFrames = 19;
	size (kPanMovieX, kPanMovieY, kPanMovieX + gPanMovie->getWidth(), kPanMovieY + (gPanMovie->getHeight()/numMovieFrames));
	panMovie = new CMovieBitmap (size, this, kPan, numMovieFrames, gPanMovie->getHeight()/numMovieFrames, gPanMovie, point);
	panMovie->setValue(effect->getParameter(kPan));
	frame->addView(panMovie);


	//--initialize the buttons----------------------------------------------

	// Destroy FX web page link
	size (kDestroyFXlinkX, kDestroyFXlinkY, kDestroyFXlinkX + gDestroyFXlink->getWidth(), kDestroyFXlinkY + (gDestroyFXlink->getHeight())/2);
	DestroyFXlink = new CKickButton (size, this, kDestroyFXlinkID, (gDestroyFXlink->getHeight())/2, gDestroyFXlink, point);
	DestroyFXlink->setValue(0.0f);
	frame->addView(DestroyFXlink);


	//--initialize the displays---------------------------------------------

	// mono merge
	size (kDisplayX, kDisplayY, kDisplayX + kDisplayWidth, kDisplayY + kDisplayHeight);
	monomergeDisplay = new CParamDisplay (size, 0, kLeftText);
	displayOffset (kDisplayX, kDisplayY);
	monomergeDisplay->setBackOffset(displayOffset);
	monomergeDisplay->setBackground(gBackground);
	monomergeDisplay->setHoriAlign(kCenterText);
	monomergeDisplay->setFont(kNormalFontSmall);
	monomergeDisplay->setFontColor(kBackgroundCColor);
	monomergeDisplay->setValue(effect->getParameter(kMonomerge));
	monomergeDisplay->setStringConvert(monomergeDisplayConvert);
	frame->addView(monomergeDisplay);

	// pan
	size.offset (0, kFaderInc-1);
	panDisplay = new CParamDisplay (size, 0, kLeftText);
	displayOffset.offset (0, kFaderInc);
	panDisplay->setBackOffset(displayOffset);
	panDisplay->setBackground(gBackground);
	panDisplay->setHoriAlign(kCenterText);
	panDisplay->setFont(kNormalFontSmall);
	panDisplay->setFontColor(kBackgroundCColor);
	panDisplay->setValue(effect->getParameter(kPan));
	panDisplay->setStringConvert(panDisplayConvert);
	frame->addView(panDisplay);


	return true;
}

//-----------------------------------------------------------------------------
void MonomakerEditor::close()
{
	if (frame)
		delete frame;
	frame = 0;

	// free some bitmaps
	if (gMonomergeFaderSlide)
		gMonomergeFaderSlide->forget();
	gMonomergeFaderSlide = 0;
	if (gPanFaderSlide)
		gPanFaderSlide->forget();
	gPanFaderSlide = 0;
	if (gFaderHandle)
		gFaderHandle->forget();
	gFaderHandle = 0;

	if (gMonomergeMovie)
		gMonomergeMovie->forget();
	gMonomergeMovie = 0;
	if (gPanMovie)
		gPanMovie->forget();
	gPanMovie = 0;

	if (gDestroyFXlink)
		gDestroyFXlink->forget();
	gDestroyFXlink = 0;
}

//-----------------------------------------------------------------------------
void MonomakerEditor::setParameter(long index, float value)
{
	if (!frame)
		return;

	// called from MonomakerEdit
	switch (index)
	{
		case kMonomerge:
			if (monomergeFader)
				monomergeFader->setValue(effect->getParameter(index));
			if (monomergeDisplay)
				monomergeDisplay->setValue(effect->getParameter(index));
			if (monomergeMovie)
				monomergeMovie->setValue(effect->getParameter(index));
			break;

		case kPan:
			if (panFader)
				panFader->setValue(effect->getParameter(index));
			if (panDisplay)
				panDisplay->setValue(effect->getParameter(index));
			if (panMovie)
				panMovie->setValue(effect->getParameter(index));
			break;

		default:
			return;
	}	

	postUpdate();
}

//-----------------------------------------------------------------------------
void MonomakerEditor::valueChanged(CDrawContext* context, CControl* control)
{
    long tag = control->getTag();

	switch (tag)
	{
		case kMonomerge:
			effect->setParameterAutomated(tag, control->getValue());
			control->update(context);
		    monomergeDisplay->update(context);
		    monomergeMovie->update(context);
		    break;
		
		case kPan:
			effect->setParameterAutomated(tag, control->getValue());
			control->update(context);
		    panDisplay->update(context);
		    panMovie->update(context);
    		break;

		// clicking on these parts of the GUI takes you to Destroy FX or SE web pages
		case kDestroyFXlinkID:
			if (control->getValue() >= 0.5f)
			{
                // create a command that tries to launch a bunch of likely browsers
                char* const browserNames =
                    "firefox 'http://www.smartelectronix.com/~destroyfx/' || "
                    "mozilla 'http://www.smartelectronix.com/~destroyfx/' || "
                    "konqueror 'http://www.smartelectronix.com/~destroyfx/' || "
                    "opera 'http://www.smartelectronix.com/~destroyfx/'";

               char* const argv[4] = { "/bin/sh", "-c", browserNames, 0 };
               const int cpid = fork();
               if (cpid == 0)
               {
                   setsid();

                   // Child process
                   execve (argv[0], argv, environ);
                   exit (0);
               }
               			
               control->update(context);
			}
			break;

		default:
			break;
	}
}
