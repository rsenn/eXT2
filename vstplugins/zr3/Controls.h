#include "Globals.h"
#include "../vstgui/vstgui.h"

#if !defined(flp_Controls_h__INCLUDED_)
#define flp_Controls_h__INCLUDED_

class CDisplay : public CControl
{
public:
    CDisplay(CRect &size,CControlListener *listener,int tag,CBitmap *background);
    ~CDisplay();
    long    getTag();
    float    getValue();
    void    setValue(float value);
    void    draw (CDrawContext *pContext);
    void    mouse (CDrawContext *pContext, CPoint& where, long button = -1);
    void    setstring(int line, char *string);
    void    setline(float value);
protected:
    float    value,last_value;
    CControlListener *my_listener;
    int        top,left,bottom,right,width,height,first,my_tag;
//    COffscreenContext    *oc;
    CDrawContext *oc;
    CBitmap    *bg;
    CDrawContext    *my_drawcontext;
    char    line1[256],line2[256],line3[256],line4[256];
    float    linevalue;
};

class CFader : public CControl
{
public:
    CFader(CRect &size,CControlListener *listener,int tag,CBitmap *bitmap,CBitmap *background, int handleheight);
    ~CFader();
    long    getTag();
    float    getValue();
    void    setValue(float value);
    void    draw (CDrawContext *pContext);
    void    mouse (CDrawContext *pContext, CPoint& where, long button = -1);
protected:
    float    value,last_value;
    CControlListener *my_listener;
    int        top,left,bottom,right,width,height,first,my_tag;
    int        my_handleheight;
//    COffscreenContext   *oc;
    CDrawContext *oc;
    CBitmap    *my_bm,*my_bg;
};

class CCircDisplay : public CControl
{
public:
    CCircDisplay(CRect &size,CControlListener *listener,int tag,CBitmap *bitmap, CBitmap *digits, CBitmap *background);
    ~CCircDisplay();
    long getTag();
    void setValue(float value);
    float getValue();
    virtual void setStringConvert (void (*convert) (float value, char *string));
    void draw (CDrawContext *pContext);
    void mouse (CDrawContext *pContext, CPoint& where, long button = -1);
protected:
    void (*stringConvert) (float value, char *string);
    float    value,last_value;
    CControlListener *my_listener;
    int        top,left,width,totalheight,numofbm,first,my_tag,digitwidth,digitheight;
    CBitmap    *my_bm,*my_bg,*my_digits;
//    COffscreenContext   *oc;
    CDrawContext *oc;
};

class CTab : public CControl
{
public:
    CTab(CRect &size,CControlListener *listener,int tag,CBitmap *bm);
    ~CTab();
    void draw (CDrawContext *pContext);
    void mouse (CDrawContext *pContext, CPoint& where, long button = -1);
};

class CTextSplash : public CSplashScreen
{
public:
    char    text[100][100];
    CTextSplash(CRect &size,
                              CControlListener *listener,
                              int     tag,
                              CBitmap *background,
                              CRect   &toDisplay,
                              CPoint  &offset);
    void draw (CDrawContext *pContext);
};

#endif // flp_Controls_h__INCLUDED_
