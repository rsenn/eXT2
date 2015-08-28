//---------------------------------------------------------------------------

#ifndef FORMH
#define FORMH
//---------------------------------------------------------------------------

#include "OPMdrv.hpp"
#include "disp.h"
#include "../vstgui/vstcontrols.h"
#include "Slider.hpp"

//---------------------------------------------------------------------------
class CForm : public CFrame,public CControlListener
{
public:
	CForm(CRect &size, void *systemWindow, AEffGUIEditor *editor,class OPMDRV *,VOPM *);
	~CForm(void);
	virtual void valueChanged (CDrawContext* context, CControl* control);
	virtual void draw(CDrawContext *context);
	virtual long onKeyDown (VstKeyCode& keyCode);
	virtual void setFocusView(CView *pView);

private:	//

	CBitmap *pKnobImage;
	CBitmap *pBtnImage;
	CBitmap *pSliderBgImage;
	CBitmap *pHdswImage;
	CDisp *pDisp;
	AEffGUIEditor *pEditor;

	void Load(void);
	void Save(void);
	CDispSlider *pSlid[4][10];//for OP
	CDispSlider *pSlid2[4];//for PROG
	CDispSlider *pSlid3[3];//for LFO
	COnOffButton *pMskBtn[4];
	COnOffButton *pAMEBtn[4];
	COnOffButton *pFileBtn[2];
	COnOffButton *pConBtn[8];
	COnOffButton *pWFBtn[4];
	COnOffButton *pNEBtn;
	COnOffButton *pHdsw;
	CTextLabel *pProgDisp;

	OPMDRV *pOPMdrv;
	VOPM *pVOPM;

};

#endif
