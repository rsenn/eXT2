/*

draw.h
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

#ifndef __DRAW__
#define __DRAW__

#include "lib.h"

// windows init

#ifdef WIN32
#include <windows.h>
#endif	

// linux init

#ifdef linux
#include <X11/Xlib.h>
Display *dp = 0;					// connection to X server
#endif	

// align

const	int	alCenter =				0;
const	int	alLeft =					1;
const	int	alTop =						1 << 1;
const	int	alRight =					1 << 2;
const	int	alBottom =				1 << 3;
const	int	alTopLeft =				1 << 4;
const	int	alTopRight =			1 << 5;
const	int	alBottomLeft =		1 << 6;
const	int	alBottomRight =		1 << 7;
const	int	alRightStretch =	1 << 8;
const	int	alBottomStretch =	1 << 9;
const	int	alHorz =					1 << 10;
const	int	alVert =					1 << 11;
struct CAlign
{
	int flag;
	int right; 
	int bottom;
};

// graphic

class CGraphic
{
	protected:
		CPoint orig;
	public:
#ifdef WIN32
		HDC dc;
		HPEN pen, oldPen;
		HBRUSH brush;
    HFONT font;
#endif	
#ifdef linux
		GC gc;
		Drawable handle;
		XFontStruct* font;		
#endif	
		int lineCol, fillCol, textCol;
		CGraphic(int device);
		~CGraphic();
		bool lock(int device);
		void unlock();
		void setLineCol(int col);
		void drawLine(int x1, int y1, int x2, int y2);
		void setFillCol(int col);
		void fillRect(CRect rc);
		void setTextCol(int col);
		void drawText(CStr text, CRect rc, int align);
};

CGraphic :: CGraphic(int device)
{
#ifdef WIN32

	pen = 0;
	brush = 0;

	HDC dc = GetDC(0);
	LOGFONT LFont;
	memset(&LFont, 0, sizeof(LFont));
  strcpy(LFont.lfFaceName, "Arial");
  LFont.lfHeight = -MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY), 72);
  font = CreateFontIndirect(&LFont);
  ReleaseDC(0, dc);

#endif	
#ifdef linux

	handle = device;
	XGCValues gcvalues;	
	gc = XCreateGC(dp, handle, 0, &gcvalues);
	font = XQueryFont(dp, XGContextFromGC(gc));

#endif	

	lineCol = 0;
	fillCol = 0;
	textCol = 0;

}

CGraphic :: ~CGraphic()
{
#ifdef WIN32
	if (pen) DeleteObject(pen);
	if (brush) DeleteObject(brush);
  if (font) DeleteObject(font);
#endif	
#ifdef linux
	XFreeGC(dp, gc);
#endif	
}

bool CGraphic :: lock(int device)
{
	orig = asPoint(0, 0);
#ifdef WIN32
	dc = (HDC)device;
#endif	
#ifdef linux
#endif
	return true;
}

void CGraphic :: unlock()
{
}

void CGraphic :: setLineCol(int col)
{
	if (col != lineCol) 
	{
		lineCol = col;		
#ifdef WIN32
		if (pen) 
		{
			SelectObject(dc, oldPen);
			DeleteObject(pen);
		}
		pen = CreatePen(PS_SOLID, 1, col);
		oldPen = (HPEN)SelectObject((HDC)dc, pen);
#endif	
#ifdef linux
#endif	
	}
}

void CGraphic :: drawLine(int x1, int y1, int x2, int y2)
{	
	x1 += orig.x;
	x2 += orig.x;
	y1 += orig.y;
	y2 += orig.y;
#ifdef WIN32
	MoveToEx(dc, x1, y1, 0);
	LineTo(dc, x2, y2);
#endif
#ifdef linux
	if (x2 > x1) x2--; else if (x1 > x2) x2++;
	if (y2 > y1) y2--; else if (y1 > y2) y2++;
	XDrawLine(dp, handle, gc, x1, y1, x2, y2);
#endif
}

void CGraphic :: setFillCol(int col)
{
	if (col != fillCol) 
	{
		fillCol = col;		
#ifdef WIN32
		if (brush)
			DeleteObject(brush);
		brush = CreateSolidBrush(col);
#endif	
#ifdef linux
#endif	
	}
}

// fill rect

void CGraphic :: fillRect(CRect rc)
{
#ifdef WIN32
	FillRect(dc, (RECT*)&rc, brush);
#endif	
#ifdef linux
	XSetForeground(dp, gc, fillCol);
	XFillRectangle(dp, handle, gc,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
#endif	
}

void CGraphic :: setTextCol(int col)
{
	if (col != textCol)
	{
		textCol = col;
#ifdef WIN32
#endif
#ifdef linux
#endif
	}
}

// draw text

void CGraphic :: drawText(CStr text, CRect rc, int align)
{

	rc.left += orig.x;
	rc.right += orig.x;
	rc.top += orig.y;
	rc.bottom += orig.y;

#ifdef WIN32

	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, textCol);

	// align

	int flags = 0;
	if (align & alLeft) flags = flags | DT_LEFT;
	else if (align & alRight) flags = flags | DT_RIGHT;
	else flags = flags | DT_CENTER;
	if (align & alTop) flags = flags | DT_TOP;
	else if (align & alBottom) flags = flags | DT_BOTTOM;
	else flags = flags | DT_VCENTER;

	// text out

	HFONT oldfont = (HFONT)SelectObject(dc, font);
	DrawText(dc, text.asChar(), -1, (RECT*)&rc, flags | DT_NOCLIP | DT_SINGLELINE | DT_NOPREFIX);
	SelectObject(dc, oldfont);	

#endif
#ifdef linux

	int x, y;
	XRectangle rect;

	// vertical alignment

	if (align & alTop) 
		y = rc.top + font->ascent;
	else if (align & alBottom) 
		y = rc.bottom - font->descent;
	else y = rc.top + ((font->ascent ) >> 1) + ((rc.bottom - rc.top) >> 1);	

	// horizontal alignment

	if (align & alLeft) 
		x = rc.left;
	else if (align & alRight) 
		x = rc.right - XTextWidth(font, text.asChar(), text.length());
	else x = rc.left + ((rc.right - rc.left) >> 1) - (XTextWidth(font, text.asChar(), text.length()) >> 1);

	// text out
	
	XSetForeground(dp, gc, textCol);
	XDrawString(dp, handle, gc, x, y, text.asChar(), text.length());  

#endif

}


#endif
