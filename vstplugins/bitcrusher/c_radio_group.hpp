/*-----------------------------------------------------------------------------

     2002 Urs Heckmann


    Note: unlike other controls, this one needs seperate bitmaps for each state


    use (implement in editor class):

    // Example implementation with 5 Buttons in a horizontal row

    // x, y, width, height <- self explanatory dimensions

    CRect size;
    size (  x, y, x + width, y + height );

    // this                        <- reference from editor class
    // kMyRadioGroup            <- the parameter id
    // 5                         <- Number of Buttons
    // 20                        <-horizontal distance of buttons
    // 0                         <-vertical distance of buttons
    // hRadioBitmap[...]Handle     <- Handles to CBitmaps...
    //                                 ... where 1-5 apply to button...
    //                                ... A/B to state off/on

    // construct
    myRadioGroup = new CRadioGroup ( size, this, kMyRadioGroup, 5, 25, 0 );

    // add CBitmaps
    myRadioGroup->addBitmap ( hRadioBitmap1AHandle, hRadioBitmap1BHandle );
    myRadioGroup->addBitmap ( hRadioBitmap2AHandle, hRadioBitmap2BHandle );
    myRadioGroup->addBitmap ( hRadioBitmap3AHandle, hRadioBitmap3BHandle );
    myRadioGroup->addBitmap ( hRadioBitmap4AHandle, hRadioBitmap4BHandle );
    myRadioGroup->addBitmap ( hRadioBitmap5AHandle, hRadioBitmap5BHandle );

    // init value
    myRadioGroup->setValue (effect->getParameter (kMyRadioGroup));

    // add to frame
    frame->addView (myRadioGroup);



-----------------------------------------------------------------------------*/
#ifndef __CRADIO__
#define __CRADIO__


// include VSTGUI
#ifndef __vstgui__
#include "../vstgui/vstgui.h"
#endif

//-----------------------------------------------------------------------------

class CRadioGroup : public CControl
{
public:
    CRadioGroup (CRect &size, CControlListener *listener, int tag,
                 int nButtons, int xOffset, int yOffset);

    virtual ~CRadioGroup ();

    virtual void draw (CDrawContext* context);
    virtual void mouse (CDrawContext *context, CPoint &where, long button = -1);

    virtual void addBitmap ( CBitmap *aBitmap, CBitmap *bBitmap );

protected:
    CBitmap **buttonsA;
    CBitmap **buttonsB;
    CRect sizeRect;

    int numButtons;        // number of buttons
    int buttonsAdded;    // counter for internal use

    int dx;                // internal layout variable
    int dy;                // dito

    int active;            // the selected button
};

#endif
