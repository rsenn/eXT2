/*

control.h
Copyright (C) 2007 jorgen www.linux-vst.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

*/

#ifndef __CONTROL__
#define __CONTROL__

#include "win.h"

class CLabel : public CCtrl
{
	public:
		CStr text;
		CLabel() : CCtrl() {}
		void paint(CGraphic *dc, CRect rc, int state);
};

void CLabel :: paint(CGraphic *dc, CRect rc, int state)
{	
	dc->setTextCol(asColor(255, 255, 255));
	dc->drawText(text, rc, alLeft);		
}

#endif
