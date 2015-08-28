/*
    This is my "standard" controls library.

    CDisplay
        The program display (program number and name, boot message)

    CFader
        Drawbars

    CCircDisplay
        The knobs with integrated number display

    CTab
        Tabs to click on...
*/

#include "Controls.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

CDisplay::CDisplay(CRect &size, CControlListener *listener, int tag, CBitmap *background) : CControl(size, listener, tag, background)
{
    my_listener=listener;
    my_tag=tag;
    bg=background;
    top=size.top;
    bottom=size.bottom;
    left=size.left;
    right=size.right;
    first=1;
    line1[0]=0;
    line2[0]=0;
    line3[0]=0;
    line4[0]=0;
    value=0;
    last_value=-2;
    linevalue=-1;
    my_drawcontext=NULL;
    oc=NULL;
}

long CDisplay::getTag()
{
    return(my_tag);
}

void CDisplay::draw(CDrawContext *pContext)
{
    last_value=value;
    my_drawcontext=pContext;
    if(first==1)
    {
//        oc=new COffscreenContext(getParent(),right-left,bottom-top,kBlackCColor);
        first=0;
    }

    oc = pContext;
    oc->offset (left, top);

    CRect rect(0,0,right-left,bottom-top);
//    CRect rect (left, top, right, bottom);
    bg->draw (oc,rect,CPoint(left,top));
    oc->setFont(kNormalFontVerySmall);
    {
        CColor mc={255,236,200,0};
        oc->setFontColor(mc);
        oc->setLineWidth(1);
        oc->setFrameColor(mc);
        mc.red=126;
        mc.green=210;
        mc.blue=204;
        oc->setFillColor(mc);
    }
    oc->drawString(line1,CRect(2,0,right-left,10),false,kLeftText);
    oc->drawString(line2,CRect(2,10,right-left,10+10),false,kLeftText);
    oc->drawString(line3,CRect(2,20,right-left,20+10),false,kLeftText);
    oc->drawString(line4,CRect(2,30,right-left,30+10),false,kLeftText);
    if(linevalue>=0)
    {
        oc->drawRect(CRect(2,42,right-left-2,48));
        oc->fillRect(CRect(3,43,4+(int)((right-left-7)*linevalue),47));
    }
//    oc->copyFrom(pContext,CRect(left,top,right,bottom),CPoint(0,0));

    oc->offset (0, 0);
    oc = 0;
}

void CDisplay::setline(float value)
{
    linevalue=value;
    this->setDirty();
}

void CDisplay::setstring(int line, char *string)
{
    if(line==1)
        strncpy(line1,string,sizeof(line1));
    else if(line==2)
        strncpy(line2,string,sizeof(line2));
    else if(line==3)
        strncpy(line3,string,sizeof(line3));
    else if(line==4)
        strncpy(line4,string,sizeof(line4));
    this->setDirty();
}

void CDisplay::setValue(float value)
{
    this->value=value;
}

float CDisplay::getValue()
{
    return(value);
}

void CDisplay::mouse (CDrawContext *pContext, CPoint& where, long button)
{
    int    x;
//    long button;

    x=where.v;

    beginEdit();
    do
    {
        button = pContext->getMouseButtons();

        pContext->getMouseLocation(where);
        if(abs(where.v-x)>10)
        {
            if(where.v<x)
            {
                if(value>0)
                {
                    setDirty(true);
                    value--;
                }
            }
            else
            {
                setDirty(true);
                value++;
            }
            x=where.v;
        }
        if (isDirty ())
            listener->valueChanged(pContext,this);
        doIdleStuff();
    }
    while(button & kLButton);
    
    endEdit ();
}

CDisplay::~CDisplay()
{
    if(oc!=NULL)
        delete oc;
}

CFader::CFader(CRect &size, CControlListener *listener, int tag, CBitmap *bitmap, CBitmap *background, int handleheight) : CControl (size, listener, tag, background)
{
    my_listener=listener;
    first=1;
    top=size.top;
    left=size.left;
    bottom=size.bottom;
    right=size.right;
    width=bitmap->getWidth();
    height=bitmap->getHeight();
    my_tag=tag;
    my_bm=bitmap;
    my_bg=background;
    my_handleheight=handleheight;
    value=0;
    last_value=-2;
    oc=NULL;
}

long CFader::getTag()
{
    return(my_tag);
}

void CFader::draw(CDrawContext *pContext)
{
    last_value=value;
    int pos = int ((1-value)*(height-my_handleheight));
    if(first==1)
    {
//        oc=new COffscreenContext(getParent(),right-left,bottom-top,kBlackCColor);
        first=0;
    }

    oc = pContext;
    oc->offset (left, top);

    CRect rect (0, 0, width, height);
    my_bm->draw (oc, rect, CPoint (0, pos));
    rect (0, height - pos, width, height);
    my_bg->draw (oc, rect, CPoint (left, bottom - pos));
//    rect (left, top, right, bottom);
//    oc->copyFrom(pContext,rect,CPoint(0,0));

    oc->offset (0, 0);
    oc = 0;
}


void CFader::mouse (CDrawContext *pContext, CPoint& where, long button)
{
    int    x,x_start;
    float    range1=.01f;
    float    range2=.001f;
    float    range;
    float    l_value=value;
    bool pressed = false;

    x_start=x=where.v;

    if (button == -1) button = pContext->getMouseButtons ();

    beginEdit();

    std::cout << std::endl;

    do
    {
        if ((button & kLButton) && !pressed)
        {
            pressed=true;
            ((AEffGUIEditor*)getParent()->getEditor())->setParameter (1000, (float)my_tag);
        }
        else if (!(button & kLButton) && pressed)
        {
            pressed=false;
            ((AEffGUIEditor*)getParent()->getEditor())->setParameter (1000, -1);
        }
        else
            break;

        if (button & kShift)
            range = range2;
        else
            range = range1;

        pContext->getMouseLocation (where);
        if (where.v != x)
        {
            x = x_start - where.v;
            value -= range*(float)x;
            if (value < 0.01f)
                value = 0;
            else if (value > 1)
                value = 1;
            x = x_start = where.v;
            if (value != l_value && my_tag < kNumParams)
                my_listener->valueChanged (pContext, this);
            l_value = value;
        }
        if (isDirty ())
            listener->valueChanged (pContext, this);

        doIdleStuff();

        button = pContext->getMouseButtons();
        
        std::cout << "." << (button & kLButton);        
    }
    while (button & kLButton);

    endEdit ();
}

void CFader::setValue(float value)
{
    this->value=value;
}

float CFader::getValue()
{
    return(value);
}

CFader::~CFader()
{
    if(oc!=NULL)
        delete    oc;
}

CCircDisplay::CCircDisplay(CRect &size, CControlListener *listener, int tag, CBitmap *bitmap, CBitmap *digits, CBitmap *background) : CControl (size, listener, tag, background)
{
    my_listener=listener;
    first=1;
    stringConvert=NULL;
    top=size.top;
    left=size.left;
    width=bitmap->getWidth();
    totalheight=bitmap->getHeight();
    numofbm=int(totalheight/width);
    my_tag=tag;
    my_bm=bitmap;
    my_digits=digits;
    my_bg=background;
    digitwidth=digits->getWidth();
    digitheight=digits->getHeight()/12;
    value=0;
    last_value=-2;
    oc=NULL;
}

void CCircDisplay::setStringConvert (void (*convert) (float value, char *string))
{
    stringConvert = convert;
}

long CCircDisplay::getTag()
{
    return(my_tag);
}


void CCircDisplay::setValue(float value)
{
    this->value=value;
}

float CCircDisplay::getValue()
{
    return(value);
}

CCircDisplay::~CCircDisplay()
{
    if(oc!=NULL)
        delete    oc;
}

void CCircDisplay::draw(CDrawContext *pContext)
{
    char    t[16];
    unsigned int x;
    int y = 0, z;
    if(first==1)
    {
//        oc=new COffscreenContext(getParent(),width,width,kBlackCColor);
        first=0;
    }

    oc = pContext;
    oc->offset (left, top);

    last_value=value;

    if(stringConvert)
        stringConvert(value,t);
    else
        sprintf(t,"%3d%",int(100*value+.001f));

    CRect rect(0,0,width,width);
    my_bg->draw (oc, rect, CPoint (left,top));
    my_bm->drawTransparent (oc, rect, CPoint(0,int(value*(numofbm-1))*width));
//    my_bm->draw (oc, rect, CPoint(0,int(value*(numofbm-1))*width));

    for(x=0;x<strlen(t);x++)
    {
        if(t[x]=='.')
        {
            int a;
            if(y>=digitwidth)
                a=y-digitwidth;
            rect(12+a,19,12+a+digitwidth,19+digitheight);
            my_digits->drawTransparent(oc,rect,CPoint(0,10*digitheight));
            continue;
        }
        else if(t[x]==' ')
        {
            y+=digitwidth;
            continue;
        }
        else if(t[x]=='%')
            z=11*digitheight;
        else if(!isdigit(t[x]))
            continue;
        else
            z=digitheight*(t[x]-'0');

        rect(12+y,18,12+y+digitwidth,18+digitheight);
        my_digits->drawTransparent(oc,rect,CPoint(0,z));
        y+=digitwidth;
    }
//    oc->copyFrom(pContext,CRect(left,top,left+width,top+width),CPoint(0,0));

    oc->offset (0, 0);
    oc = 0;
}

void CCircDisplay::mouse (CDrawContext *pContext, CPoint& where, long button)
{
    int    x,x_start;
//    long button;
    float    range1=.01f;
    float    range2=.001f;
    float    range;
    float    l_value=value;
    bool    pressed=false;

    x_start=x=where.v;

    if (button == -1) button = pContext->getMouseButtons ();

    beginEdit();

//    do
    {
        if((button&kLButton) && !pressed)
        {
            pressed=true;
            ((AEffGUIEditor*)getParent()->getEditor())->setParameter(1000,(float)my_tag);
        }
        else if(!(button&kLButton) && pressed)
        {
            pressed=false;
            ((AEffGUIEditor*)getParent()->getEditor())->setParameter(1000,-1);
        }
        else
            return;

        if(button & kShift)
            range=range2;
        else
            range=range1;

        pContext->getMouseLocation(where);
        if(where.v != x)
        {
            x=x_start-where.v;
            value+=range*(float)x;
            if(value<0.01f)
                value=0;
            else if(value>1)
                value=1;
            x=x_start=where.v;
//            if(value!=l_value && my_tag<kNumParams && isDirty())
            if(value!=l_value && my_tag<kNumParams)
            {
                setDirty(true);
                my_listener->valueChanged(pContext, (CControl *) this);
            }
            l_value=value;
        }
        if (isDirty ())
            listener->valueChanged (pContext, this);

//        pContext->getMouseLocation (where);
//        doIdleStuff();
//        button = pContext->getMouseButtons();
    }
//    while(button & kLButton);

    endEdit ();
}

CTextSplash::CTextSplash(CRect &size,CControlListener *listener, int tag, CBitmap *background, CRect   &toDisplay, CPoint  &offset) : CSplashScreen(size,listener,tag,background,toDisplay,offset)
{
}

void CTextSplash::draw(CDrawContext *pContext)
{
    if (value && pBackground)
    {
        pBackground->draw (pContext, toDisplay, offset);
        pContext->setFont(kNormalFontSmaller);
        {
            int    x;
            CColor mc={0,0,0,0};
            pContext->setFontColor(mc);

            for(x=0;x<100;x++)
            {
                pContext->drawString(text[x],CRect(158,x*10,250,10+x*10),false,kLeftText);
            }
        }
    }
}

CTab::CTab(CRect &size,CControlListener *listener,int tag,CBitmap *bm) : CControl(size, listener, tag, bm)
{
}

CTab::~CTab()
{
}

void CTab::draw(CDrawContext *pContext)
{
}

void CTab::mouse(CDrawContext *pContext, CPoint& where, long button)
{
    if (button == -1)
        button = pContext->getMouseButtons();

    if (button != -1)
        ((AEffGUIEditor*)getParent()->getEditor())->setParameter (tag, 1);
}
