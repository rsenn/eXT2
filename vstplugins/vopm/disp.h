<<<<<<< HEAD
//---------------------------------------------------------------------------

#ifndef DISPH
#define DISPH
//---------------------------------------------------------------------------

#include "OPMdrv.hpp"
#include "../vstgui/vstcontrols.h"

class CDisp : public CView{
public:
	~CDisp(void);
	CDisp(CRect &size,OPMDRV *pO);
	void draw (CDrawContext *context);
	void EGBPaint(CDrawContext *context);
	void ConPaint(CDrawContext *context,unsigned char con);
	void mouse(CDrawContext *context,CPoint &where,long button);
private:
	int EGmode;
	CDrawContext *pCOffScreen;
	CBitmap *pConImage;
	CBitmap *pBgImage;
	OPMDRV *pOPMdrv;
};

#endif
=======
//---------------------------------------------------------------------------

#ifndef DISPH
#define DISPH
//---------------------------------------------------------------------------

#include "OPMdrv.hpp"
#include "../vstgui/vstcontrols.h"

class CDisp : public CView{
public:
	~CDisp(void);
	CDisp(CRect &size,OPMDRV *pO);
	void draw (CDrawContext *context);
	void EGBPaint(CDrawContext *context);
	void ConPaint(CDrawContext *context,unsigned char con);
	void mouse(CDrawContext *context,CPoint &where,long button);
private:
	int EGmode;
	CDrawContext *pCOffScreen;
	CBitmap *pConImage;
	CBitmap *pBgImage;
	OPMDRV *pOPMdrv;
};

#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
