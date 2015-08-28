//VOPMEdit.hpp
//
//
#ifndef __VOPMEdit_HEADER__
#define __VOPMEdit_HEADER__

#ifndef __OPMDRV__
#include "OPMdrv.hpp"
#endif
#include "VOPM.hpp"

#include "../vstgui/vstgui.h"

class VOPMEdit : public AEffGUIEditor
{
public:
	VOPMEdit(VOPM *effect);
	virtual ~VOPMEdit();
	virtual long getRect(ERect **);
	virtual long open(void *ptr);
	virtual void close();

#if MAC
	virtual void top() {}
	virtual void sleep() {}
#endif
	void update();
	long getTag();

private:
	class CHDATA *pVoTbl;
	class OPMDRV *pOPMdrv;
	class VOPM *Effect;
	CBitmap *hBackground;
};

#endif
