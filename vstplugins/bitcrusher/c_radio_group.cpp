/*-----------------------------------------------------------------------------

 2002 Urs Heckmann

    see header for details

-----------------------------------------------------------------------------*/


#ifndef __CRADIO__
#include "c_radio_group.hpp"
#endif

CRadioGroup::CRadioGroup (CRect &size, CControlListener *listener, int tag,
                          int nButtons, int xOffset, int yOffset)
  : CControl (size, listener, tag)
{
    numButtons = nButtons;
    dx = xOffset;
    dy = yOffset;
    active = 0;
    sizeRect = size;
    buttonsAdded = 0;
    buttonsA = new CBitmap*[ numButtons ];
    buttonsB = new CBitmap*[ numButtons ];

    for ( int i = 0; i < numButtons; i++ )
    {
        buttonsA[ i ] = 0;
        buttonsB[ i ] = 0;
    }
}

CRadioGroup::~CRadioGroup ()
{
    for ( int i = 0; i < numButtons; i++ )
    {
        buttonsA[ i ] = 0;
        buttonsB[ i ] = 0;
    }
}

void CRadioGroup::draw (CDrawContext* context)
{
    CRect buttonPos;

    active = (int)( getValue() * (float)( numButtons - 1 ) );

    for ( int i = 0; i < numButtons; i ++ )
    {
        buttonPos.left = sizeRect.left + i * dx;
        buttonPos.top = sizeRect.top + i * dy;
        buttonPos.right = buttonPos.left + buttonsA[ i ]->getWidth();
        buttonPos.bottom = buttonPos.top + buttonsA[ i ]->getHeight();

        if ( active == i )
            buttonsB[ i ]->draw( context, buttonPos, CPoint (0, 0) );
        else
            buttonsA[ i ]->draw( context, buttonPos, CPoint (0, 0) );
    }

    context->setFrameColor (kRedCColor);
    context->drawRect (sizeRect);
}
void CRadioGroup::mouse (CDrawContext *context, CPoint &where, long button)
{
    int found = -1;

    CRect buttonPos;

    for ( int i = 0; i < numButtons; i ++ )
    {
        buttonPos.left = sizeRect.left + i * dx;
        buttonPos.top = sizeRect.top + i * dy;
        buttonPos.right = buttonPos.left + buttonsA[ i ]->getWidth();
        buttonPos.bottom = buttonPos.top + buttonsA[ i ]->getHeight();

        if ( where.isInside (buttonPos) )
        {
            found = i;
            break;
        }
    }

    if ( found >= 0 && button != -1 )
    {
        if ( found != active )
        {
            active = found;

            setValue ( (float)found / (float)(numButtons - 1 ) );

            listener->valueChanged ( context, this );
            draw (context);
        }

        do
        {
            doIdleStuff ();
        }
        while (context->getMouseButtons ());
    }
}

void CRadioGroup::addBitmap ( CBitmap *aBitmap, CBitmap *bBitmap )
{
    if ( buttonsAdded < numButtons )
    {
        buttonsA[ buttonsAdded ] = aBitmap;
        buttonsB[ buttonsAdded++ ] = bBitmap;
    }
}




