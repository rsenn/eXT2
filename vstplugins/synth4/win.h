/*

win.h
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

<<<<<<< HEAD
#ifndef __GUI__
#define __GUI__

#include "draw.h"

// mouse buttons and mod keys

const int buNone		= 0;
const int buLeft		= 1;
const int buRight		= 1 << 1;
const int buMiddle	= 1 << 2;
const int buShift		= 1 << 3;
const int buCtrl		= 1 << 4;
const int buAlt			= 1 << 5;
const int buDouble	= 1 << 6;
const int buHoover	= 1 << 7;

int lastBtn = 0;

// return color value from RGB

int asColor(int r, int g, int b)
{
#ifdef linux
	XColor xcol;
	xcol.red = r << 8;
	xcol.green = g << 8; 
	xcol.blue = b << 8;
  xcol.flags = (DoRed or DoGreen or DoBlue);
	XAllocColor(dp, XDefaultColormap(dp, 0), &xcol);
	return xcol.pixel;
#endif
#ifdef WIN32
	return RGB(r, g, b);
#endif
}

// standard colors
/*
const int clBlack = asColor(0, 0, 0);
const int clWhite = asColor(255, 255, 255);
const int clRed = asColor(128, 0, 0);
const int clGreen = asColor(0, 128, 0);
const int clBlue= asColor(0, 0, 128);
*/

void showStr(char* str)
{
#ifdef WIN32
	MessageBox(0, str, "", 0);
#endif
#ifdef linux
	printf("%s\n", str);
#endif
}

// control

class CGroup;
class CWin;

class CCtrl
{	
	public:
		CGroup* parent;
		CWin* owner;		
		bool visible;
		int left, top, width, height;
		CCtrl();
		void setBounds(int x, int y, int w, int h) 
			{ left = x; top = y; width = w; height = h; }
		virtual void paint(CGraphic *dc, CRect rc, int state);
		virtual void mouseDown(int x, int y, int btn);
		virtual void mouseMove(int x, int y, int btn);
		virtual void mouseUp(int x, int y, int btn);
		virtual void redraw();
};

// group

class CGroup : public CCtrl
{
	public:
		CList* controls;
		CGroup();
		~CGroup();
		virtual void addControl(CCtrl* co);
		virtual void resized();
};

// win

class CWin : public CGroup
{	
	protected:
#ifdef WIN32
		HWND handle;
#endif	
#ifdef linux
		Window handle;
#endif		
		CCtrl* mouseCtrl;
	public:
		CWin();
		~CWin();
		CGraphic* dc;
		int getHandle() 
			{ return (int) handle; }
		void show();
		void hide();
		void paint(int device, CRect rc);
		void setPos(int x, int y);
		void setSize(int x, int y);
		CPoint getSize();
		void resized();
		virtual void addControl(CCtrl* co);
		virtual void mouseDown(int x, int y, int btn);
		virtual void mouseMove(int x, int y, int btn);
		virtual void mouseUp(int x, int y, int btn);
		virtual void redraw();
};

CCtrl :: CCtrl()
{
	parent = 0;
	owner = 0;
	visible = true;
}

void CCtrl :: mouseDown(int x, int y, int btn)
{
}

void CCtrl :: mouseMove(int x, int y, int btn)
{
}

void CCtrl :: mouseUp(int x, int y, int btn)
{
}

void CCtrl :: paint(CGraphic *dc, CRect rc, int state)
{
	dc->setFillCol(asColor(0, 128, 0));	
	dc->fillRect(rc);
}

void CCtrl :: redraw()
{
	CPoint p;
	CRect rc;
	if (owner && visible)
	{
		p = asPoint(left, top);
		rc = asRect(p.x, p.y, p.x + width, p.y + height);
#ifdef WIN32
		InvalidateRect((HWND)owner->getHandle(), (RECT*)&rc, false);
		UpdateWindow((HWND)owner->getHandle());
#endif
#ifdef linux
		XClearArea(dp, owner->getHandle(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
		XSync(dp, false);
#endif
	}

}

CGroup :: CGroup()
	: CCtrl()
{	
	controls = new CList;
}

CGroup :: ~CGroup()
{
	delete controls;
}

void CGroup :: addControl(CCtrl* co)
{
	controls->add(co);
}

void CGroup :: resized()
{
}

// windows event handler

#ifdef WIN32

HINSTANCE hInstance;

int modKeys(int wParam)
{
	int result = buNone;
  if (wParam & MK_SHIFT) 
    result |= buShift;
  if (wParam & MK_CONTROL) 
    result |= buCtrl;
  if (GetKeyState(VK_MENU) < 0) 
    result |= buAlt;
	return result;
}

LRESULT CALLBACK wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	int btn;

  CWin* wnd = (CWin*) GetWindowLong(hWnd, GWL_USERDATA);
	if (wnd == 0)
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
		
    case WM_GETDLGCODE:
      result = DLGC_WANTALLKEYS;
			break;

		case WM_PAINT:
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
      wnd->paint((int)ps.hdc, asRect(ps.rcPaint.left, ps.rcPaint.top, 
				ps.rcPaint.right, ps.rcPaint.bottom));
      EndPaint(hWnd, &ps);
      return 1;
			break;
		
    case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:			
			switch (message) 
			{
				case WM_LBUTTONDOWN: btn = buLeft; break;
				case WM_MBUTTONDOWN: btn = buMiddle; break;
				case WM_RBUTTONDOWN: btn = buRight; break;
			}
			// todo: double-click
			lastBtn = btn;
			wnd->mouseDown(short(LOWORD(lParam)), short(HIWORD(lParam)), btn | modKeys(wParam));
			break;

    case WM_MOUSEMOVE: 
    	wnd->mouseMove(short(LOWORD(lParam)), short(HIWORD(lParam)), lastBtn | modKeys(wParam));
    	return 1;
    break;

    case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
			switch (message) 
			{
				case WM_LBUTTONUP: btn = buLeft; break;
				case WM_MBUTTONUP: btn = buMiddle; break;
				case WM_RBUTTONUP: btn = buRight; break;
			}
			wnd->mouseUp(short(LOWORD(lParam)), short(HIWORD(lParam)), btn | modKeys(wParam));
			break;

    case WM_SIZE:
    	wnd->resized();
    	return 0;
    break;

    default:
      result = DefWindowProc(hWnd, message, wParam, lParam);

	}

	return result;

}

#endif	

// linux event handler

#ifdef linux

bool xerror;
int errorHandler(Display *dp, XErrorEvent *e)
{
	xerror = true;
}
int getProperty(Window handle, Atom atom)
{ 
	int result = 0, userSize;
	unsigned long bytes, userCount;
	unsigned char *data;
 	Atom userType;
	xerror = false;
	XErrorHandler olderrorhandler = XSetErrorHandler(errorHandler);
 	XGetWindowProperty(dp, handle, atom, 0,	1, false, AnyPropertyType, 
	 	&userType,	&userSize, &userCount, &bytes, &data);
	if (xerror == false && userCount == 1)
 		result = *(int*)data;
	XSetErrorHandler(olderrorhandler);
	return result;
}

int modKeys(int state)
{
	int result;
	if (state & 1) result += buShift;
	if (state & 4) result += buCtrl;
	if (state & 8) result += buAlt;	
	return result;
}
=======
#ifndef __GUI__
#define __GUI__

#include "draw.h"

// mouse buttons and mod keys

const int buNone		= 0;
const int buLeft		= 1;
const int buRight		= 1 << 1;
const int buMiddle	= 1 << 2;
const int buShift		= 1 << 3;
const int buCtrl		= 1 << 4;
const int buAlt			= 1 << 5;
const int buDouble	= 1 << 6;
const int buHoover	= 1 << 7;

int lastBtn = 0;

// return color value from RGB

int asColor(int r, int g, int b)
{
#ifdef linux
	XColor xcol;
	xcol.red = r << 8;
	xcol.green = g << 8; 
	xcol.blue = b << 8;
  xcol.flags = (DoRed or DoGreen or DoBlue);
	XAllocColor(dp, XDefaultColormap(dp, 0), &xcol);
	return xcol.pixel;
#endif
#ifdef WIN32
	return RGB(r, g, b);
#endif
}

// standard colors
/*
const int clBlack = asColor(0, 0, 0);
const int clWhite = asColor(255, 255, 255);
const int clRed = asColor(128, 0, 0);
const int clGreen = asColor(0, 128, 0);
const int clBlue= asColor(0, 0, 128);
*/

void showStr(char* str)
{
#ifdef WIN32
	MessageBox(0, str, "", 0);
#endif
#ifdef linux
	printf("%s\n", str);
#endif
}

// control

class CGroup;
class CWin;

class CCtrl
{	
	public:
		CGroup* parent;
		CWin* owner;		
		bool visible;
		int left, top, width, height;
		CCtrl();
		void setBounds(int x, int y, int w, int h) 
			{ left = x; top = y; width = w; height = h; }
		virtual void paint(CGraphic *dc, CRect rc, int state);
		virtual void mouseDown(int x, int y, int btn);
		virtual void mouseMove(int x, int y, int btn);
		virtual void mouseUp(int x, int y, int btn);
		virtual void redraw();
};

// group

class CGroup : public CCtrl
{
	public:
		CList* controls;
		CGroup();
		~CGroup();
		virtual void addControl(CCtrl* co);
		virtual void resized();
};

// win

class CWin : public CGroup
{	
	protected:
#ifdef WIN32
		HWND handle;
#endif	
#ifdef linux
		Window handle;
#endif		
		CCtrl* mouseCtrl;
	public:
		CWin();
		~CWin();
		CGraphic* dc;
		int getHandle() 
			{ return (int) handle; }
		void show();
		void hide();
		void paint(int device, CRect rc);
		void setPos(int x, int y);
		void setSize(int x, int y);
		CPoint getSize();
		void resized();
		virtual void addControl(CCtrl* co);
		virtual void mouseDown(int x, int y, int btn);
		virtual void mouseMove(int x, int y, int btn);
		virtual void mouseUp(int x, int y, int btn);
		virtual void redraw();
};

CCtrl :: CCtrl()
{
	parent = 0;
	owner = 0;
	visible = true;
}

void CCtrl :: mouseDown(int x, int y, int btn)
{
}

void CCtrl :: mouseMove(int x, int y, int btn)
{
}

void CCtrl :: mouseUp(int x, int y, int btn)
{
}

void CCtrl :: paint(CGraphic *dc, CRect rc, int state)
{
	dc->setFillCol(asColor(0, 128, 0));	
	dc->fillRect(rc);
}

void CCtrl :: redraw()
{
	CPoint p;
	CRect rc;
	if (owner && visible)
	{
		p = asPoint(left, top);
		rc = asRect(p.x, p.y, p.x + width, p.y + height);
#ifdef WIN32
		InvalidateRect((HWND)owner->getHandle(), (RECT*)&rc, false);
		UpdateWindow((HWND)owner->getHandle());
#endif
#ifdef linux
		XClearArea(dp, owner->getHandle(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
		XSync(dp, false);
#endif
	}

}

CGroup :: CGroup()
	: CCtrl()
{	
	controls = new CList;
}

CGroup :: ~CGroup()
{
	delete controls;
}

void CGroup :: addControl(CCtrl* co)
{
	controls->add(co);
}

void CGroup :: resized()
{
}

// windows event handler

#ifdef WIN32

HINSTANCE hInstance;

int modKeys(int wParam)
{
	int result = buNone;
  if (wParam & MK_SHIFT) 
    result |= buShift;
  if (wParam & MK_CONTROL) 
    result |= buCtrl;
  if (GetKeyState(VK_MENU) < 0) 
    result |= buAlt;
	return result;
}

LRESULT CALLBACK wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	int btn;

  CWin* wnd = (CWin*) GetWindowLong(hWnd, GWL_USERDATA);
	if (wnd == 0)
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
		
    case WM_GETDLGCODE:
      result = DLGC_WANTALLKEYS;
			break;

		case WM_PAINT:
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
      wnd->paint((int)ps.hdc, asRect(ps.rcPaint.left, ps.rcPaint.top, 
				ps.rcPaint.right, ps.rcPaint.bottom));
      EndPaint(hWnd, &ps);
      return 1;
			break;
		
    case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:			
			switch (message) 
			{
				case WM_LBUTTONDOWN: btn = buLeft; break;
				case WM_MBUTTONDOWN: btn = buMiddle; break;
				case WM_RBUTTONDOWN: btn = buRight; break;
			}
			// todo: double-click
			lastBtn = btn;
			wnd->mouseDown(short(LOWORD(lParam)), short(HIWORD(lParam)), btn | modKeys(wParam));
			break;

    case WM_MOUSEMOVE: 
    	wnd->mouseMove(short(LOWORD(lParam)), short(HIWORD(lParam)), lastBtn | modKeys(wParam));
    	return 1;
    break;

    case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
			switch (message) 
			{
				case WM_LBUTTONUP: btn = buLeft; break;
				case WM_MBUTTONUP: btn = buMiddle; break;
				case WM_RBUTTONUP: btn = buRight; break;
			}
			wnd->mouseUp(short(LOWORD(lParam)), short(HIWORD(lParam)), btn | modKeys(wParam));
			break;

    case WM_SIZE:
    	wnd->resized();
    	return 0;
    break;

    default:
      result = DefWindowProc(hWnd, message, wParam, lParam);

	}

	return result;

}

#endif	

// linux event handler

#ifdef linux

bool xerror;
int errorHandler(Display *dp, XErrorEvent *e)
{
	xerror = true;
}
int getProperty(Window handle, Atom atom)
{ 
	int result = 0, userSize;
	unsigned long bytes, userCount;
	unsigned char *data;
 	Atom userType;
	xerror = false;
	XErrorHandler olderrorhandler = XSetErrorHandler(errorHandler);
 	XGetWindowProperty(dp, handle, atom, 0,	1, false, AnyPropertyType, 
	 	&userType,	&userSize, &userCount, &bytes, &data);
	if (xerror == false && userCount == 1)
 		result = *(int*)data;
	XSetErrorHandler(olderrorhandler);
	return result;
}

int modKeys(int state)
{
	int result;
	if (state & 1) result += buShift;
	if (state & 4) result += buCtrl;
	if (state & 8) result += buAlt;	
	return result;
}
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a

void eventProc(XEvent* ev)
{
	int btn;
	CRect rc;
<<<<<<< HEAD
	
	CWin* win = (CWin*) getProperty(ev->xany.window, XInternAtom(dp, "_this", false));
	if (win == 0)
		return;
	
	switch (ev->type) 
	{
=======
	
	CWin* win = (CWin*) getProperty(ev->xany.window, XInternAtom(dp, "_this", false));
	if (win == 0)
		return;
	
	switch (ev->type) 
	{
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
		
		case ButtonPress: 
			btn = 0;
			switch (ev->xbutton.button)
			{
				case 1: btn = buLeft; break;
				case 2: btn = buMiddle; break;
<<<<<<< HEAD
				case 3: btn = buRight; break;
				//case 4: win->mouseWheel(ev->xbutton.x, ev->xbutton.y, temp, 1); break;
=======
				case 3: btn = buRight; break;
				//case 4: win->mouseWheel(ev->xbutton.x, ev->xbutton.y, temp, 1); break;
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
				//case 5: win->mouseWheel(ev->xbutton.x, ev->xbutton.y, temp, -1); break;
			}			
			if (btn)
			{
				// todo: double-click
				win->mouseDown(ev->xbutton.x, ev->xbutton.y, btn | modKeys(ev->xbutton.state));
			}			
<<<<<<< HEAD
			lastBtn = btn;
			break;
		
		
		case MotionNotify: 
			win->mouseMove(ev->xbutton.x, ev->xbutton.y, lastBtn | modKeys(ev->xbutton.state));
			break;
			
		case ButtonRelease:		
			win->mouseUp(ev->xbutton.x, ev->xbutton.y, btn | modKeys(ev->xbutton.state));			
			lastBtn = 0;
		break;
		
		case Expose:
			rc = asRect(ev->xexpose.x, ev->xexpose.y,
				ev->xexpose.x + ev->xexpose.width, ev->xexpose.y + ev->xexpose.height);
    	while (XCheckTypedWindowEvent(dp, ev->xexpose.window, Expose, ev))
    	{
				rc.left = asMin(rc.left, ev->xexpose.x);
				rc.top = asMin(rc.top, ev->xexpose.y);
				rc.right = asMax(rc.right, ev->xexpose.x + ev->xexpose.width);
				rc.bottom = asMax(rc.bottom, ev->xexpose.y + ev->xexpose.height);
			}
			win->paint(0, rc);				
		break;

		case ConfigureNotify: 
			win->resized();
			break;

	}
	
}

#endif	

CWin :: CWin()
	: CGroup()
{

	visible = false;
	mouseCtrl = 0;

#ifdef WIN32

	// register window class
	
	char* classname = "_win";
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = &wndproc;
  wc.hInstance = hInstance;
  wc.lpszClassName = classname;	
  RegisterClass(&wc);

	// create window

  handle = CreateWindowEx(
  	WS_EX_TOOLWINDOW, classname, 0, WS_POPUP,  
		0, 0, 100, 100, 0,	0, hInstance, 0);

  SetWindowLong(handle, GWL_USERDATA, int(this));	// store pointer to "this"
  DragAcceptFiles(handle, true);									// enable as drag & drop target

#endif	
#ifdef linux

	// window attributes

	XSetWindowAttributes swa;
	swa.override_redirect = true; 
	swa.background_pixmap = None;
	swa.colormap = 0;
	swa.event_mask = 
		StructureNotifyMask | ExposureMask |ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask |	PropertyChangeMask;

	// create window

	handle = XCreateWindow(
		dp, XDefaultRootWindow(dp), 0, 0, 100, 100, 
		CopyFromParent, CopyFromParent, InputOutput, CopyFromParent, 
		CWBackPixmap | CWEventMask, &swa);
=======
			lastBtn = btn;
			break;
		
		
		case MotionNotify: 
			win->mouseMove(ev->xbutton.x, ev->xbutton.y, lastBtn | modKeys(ev->xbutton.state));
			break;
			
		case ButtonRelease:		
			win->mouseUp(ev->xbutton.x, ev->xbutton.y, btn | modKeys(ev->xbutton.state));			
			lastBtn = 0;
		break;
		
		case Expose:
			rc = asRect(ev->xexpose.x, ev->xexpose.y,
				ev->xexpose.x + ev->xexpose.width, ev->xexpose.y + ev->xexpose.height);
    	while (XCheckTypedWindowEvent(dp, ev->xexpose.window, Expose, ev))
    	{
				rc.left = asMin(rc.left, ev->xexpose.x);
				rc.top = asMin(rc.top, ev->xexpose.y);
				rc.right = asMax(rc.right, ev->xexpose.x + ev->xexpose.width);
				rc.bottom = asMax(rc.bottom, ev->xexpose.y + ev->xexpose.height);
			}
			win->paint(0, rc);				
		break;

		case ConfigureNotify: 
			win->resized();
			break;

	}
	
}

#endif	

CWin :: CWin()
	: CGroup()
{

	visible = false;
	mouseCtrl = 0;

#ifdef WIN32

	// register window class
	
	char* classname = "_win";
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = &wndproc;
  wc.hInstance = hInstance;
  wc.lpszClassName = classname;	
  RegisterClass(&wc);

	// create window

  handle = CreateWindowEx(
  	WS_EX_TOOLWINDOW, classname, 0, WS_POPUP,  
		0, 0, 100, 100, 0,	0, hInstance, 0);

  SetWindowLong(handle, GWL_USERDATA, int(this));	// store pointer to "this"
  DragAcceptFiles(handle, true);									// enable as drag & drop target

#endif	
#ifdef linux

	// window attributes

	XSetWindowAttributes swa;
	swa.override_redirect = true; 
	swa.background_pixmap = None;
	swa.colormap = 0;
	swa.event_mask = 
		StructureNotifyMask | ExposureMask |ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask |	PropertyChangeMask;

	// create window

	handle = XCreateWindow(
		dp, XDefaultRootWindow(dp), 0, 0, 100, 100, 
		CopyFromParent, CopyFromParent, InputOutput, CopyFromParent, 
		CWBackPixmap | CWEventMask, &swa);
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a

	// remove window caption/frame
	
	#define MWM_HINTS_DECORATIONS (1L << 1)
	#define PROP_MOTIF_WM_HINTS_ELEMENTS 5
	typedef struct
	{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long          inputMode;
    unsigned long status;

	} PropMotifWmHints; 	
	PropMotifWmHints motif_hints;
<<<<<<< HEAD
	motif_hints.flags = MWM_HINTS_DECORATIONS;
  motif_hints.decorations = 0;	
  Atom prop = XInternAtom( dp, "_MOTIF_WM_HINTS", True );	
  XChangeProperty( dp, handle, prop, prop, 32, PropModeReplace,            
  	(unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);

=======
	motif_hints.flags = MWM_HINTS_DECORATIONS;
  motif_hints.decorations = 0;	
  Atom prop = XInternAtom( dp, "_MOTIF_WM_HINTS", True );	
  XChangeProperty( dp, handle, prop, prop, 32, PropModeReplace,            
  	(unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);

>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
	// set user data
	
	Atom atom;
	
	void* data = this;
	atom = XInternAtom(dp, "_this", false);
<<<<<<< HEAD
 	XChangeProperty(dp, handle, atom, atom,	32, 				// store pointer to "this"
=======
 	XChangeProperty(dp, handle, atom, atom,	32, 				// store pointer to "this"
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
 		PropModeReplace, (unsigned char*)&data,	1);

	data = (void*)&eventProc;
	atom = XInternAtom(dp, "_XEventProc", false);				// store pointer to event proc
<<<<<<< HEAD
 	XChangeProperty(dp, handle, atom, atom,	32, 
 		PropModeReplace, (unsigned char*)&data,	1);

#endif

	dc = new CGraphic((int)handle);

}

CWin :: ~CWin()
{
	hide();
	delete dc;
#ifdef WIN32
	DestroyWindow(handle);
#endif
#ifdef linux
	XDestroyWindow(dp, handle);
#endif
}

void CWin :: mouseDown(int x, int y, int btn)
{	
	CCtrl* co;
	void* next = 0;
	while (co = (CCtrl*)controls->next(&next))
	{		
		if (x >= co->left && x < co->left + co->width &&
				y >= co->top && y < co->top + co->height)
		{
			mouseCtrl = co;
			co->mouseDown(x - co->left, y - co->top, btn);
			break;
		}					
	}
}

void CWin :: mouseMove(int x, int y, int btn)
{
	if (mouseCtrl)
		mouseCtrl->mouseMove(x - mouseCtrl->left, y - mouseCtrl->top, btn);
}

void CWin :: mouseUp(int x, int y, int btn)
{
	if (mouseCtrl)
	{
		mouseCtrl->mouseUp(x - mouseCtrl->left, y - mouseCtrl->top, btn);
		mouseCtrl = 0;
	}
}

void CWin :: redraw()
{
}

// show window

void CWin :: show()
{
	if (!visible)
	{
#ifdef WIN32
	SetWindowPos(handle, 0, 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
#endif	
#ifdef linux
		XMapRaised(dp, handle);
#endif		
		visible = true;
	}
}

void CWin :: hide()
{
	if (visible) 
	{
#ifdef WIN32
		ShowWindow(handle, SW_HIDE);
#endif	
#ifdef linux
		XUnmapWindow(dp, handle);
#endif	
		visible = false;
	}
}

// paint window

void CWin :: paint(int device, CRect rc)
=======
 	XChangeProperty(dp, handle, atom, atom,	32, 
 		PropModeReplace, (unsigned char*)&data,	1);

#endif

	dc = new CGraphic((int)handle);

}

CWin :: ~CWin()
{
	hide();
	delete dc;
#ifdef WIN32
	DestroyWindow(handle);
#endif
#ifdef linux
	XDestroyWindow(dp, handle);
#endif
}

void CWin :: mouseDown(int x, int y, int btn)
{	
	CCtrl* co;
	void* next = 0;
	while (co = (CCtrl*)controls->next(&next))
	{		
		if (x >= co->left && x < co->left + co->width &&
				y >= co->top && y < co->top + co->height)
		{
			mouseCtrl = co;
			co->mouseDown(x - co->left, y - co->top, btn);
			break;
		}					
	}
}

void CWin :: mouseMove(int x, int y, int btn)
{
	if (mouseCtrl)
		mouseCtrl->mouseMove(x - mouseCtrl->left, y - mouseCtrl->top, btn);
}

void CWin :: mouseUp(int x, int y, int btn)
{
	if (mouseCtrl)
	{
		mouseCtrl->mouseUp(x - mouseCtrl->left, y - mouseCtrl->top, btn);
		mouseCtrl = 0;
	}
}

void CWin :: redraw()
{
}

// show window

void CWin :: show()
{
	if (!visible)
	{
#ifdef WIN32
	SetWindowPos(handle, 0, 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
#endif	
#ifdef linux
		XMapRaised(dp, handle);
#endif		
		visible = true;
	}
}

void CWin :: hide()
{
	if (visible) 
	{
#ifdef WIN32
		ShowWindow(handle, SW_HIDE);
#endif	
#ifdef linux
		XUnmapWindow(dp, handle);
#endif	
		visible = false;
	}
}

// paint window

void CWin :: paint(int device, CRect rc)
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
{

	if (dc->lock(device))
	{
	
<<<<<<< HEAD
		// bkgnd

		dc->setFillCol(asColor(128, 128, 96));
		dc->fillRect(rc);

		// controls

		CCtrl* co;
		void* next = 0;
		while (co = (CCtrl*)controls->next(&next))
		{		
			rc = asRect(co->left, co->top, 
				co->left + co->width, co->top + co->height);
			co->paint(dc, rc, 0); // todo: state
		}
	
		dc->unlock();

	}

}

// set position of window

void CWin :: setPos(int x, int y)
{
	left = x;
	top = y;
#ifdef linux
	XWindowChanges attr;
	attr.x = x;
	attr.y = y;
	XConfigureWindow(dp, handle, CWX | CWY, &attr);
#endif
#ifdef WIN32
  SetWindowPos(handle, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
#endif
}

// set size of window

void CWin :: setSize(int x, int y)
{
	width = x;
	height = y;
#ifdef WIN32
  SetWindowPos(handle, 0, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
#endif
#ifdef linux
	XResizeWindow(dp, handle, x, y);
#endif
}

// return size of window

CPoint CWin :: getSize()
{
	CPoint result;
	result.x = width;
	result.y = height;		
	return result;
}

// window was resized
=======
		// bkgnd

		dc->setFillCol(asColor(128, 128, 96));
		dc->fillRect(rc);

		// controls

		CCtrl* co;
		void* next = 0;
		while (co = (CCtrl*)controls->next(&next))
		{		
			rc = asRect(co->left, co->top, 
				co->left + co->width, co->top + co->height);
			co->paint(dc, rc, 0); // todo: state
		}
	
		dc->unlock();

	}

}

// set position of window

void CWin :: setPos(int x, int y)
{
	left = x;
	top = y;
#ifdef linux
	XWindowChanges attr;
	attr.x = x;
	attr.y = y;
	XConfigureWindow(dp, handle, CWX | CWY, &attr);
#endif
#ifdef WIN32
  SetWindowPos(handle, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
#endif
}

// set size of window

void CWin :: setSize(int x, int y)
{
	width = x;
	height = y;
#ifdef WIN32
  SetWindowPos(handle, 0, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
#endif
#ifdef linux
	XResizeWindow(dp, handle, x, y);
#endif
}

// return size of window

CPoint CWin :: getSize()
{
	CPoint result;
	result.x = width;
	result.y = height;		
	return result;
}

// window was resized
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a

void CWin :: resized()
{
	// todo: get size of window
<<<<<<< HEAD
	CGroup :: resized();
}

void reparentWindow(int handle, int parent)
{
#ifdef WIN32	
	SetWindowLong ((HWND)handle, GWL_STYLE,
		(GetWindowLong((HWND)handle, GWL_STYLE) &~ WS_POPUP) | WS_CHILD);
	SetParent((HWND)handle, HWND(parent));
#endif
=======
	CGroup :: resized();
}

void reparentWindow(int handle, int parent)
{
#ifdef WIN32	
	SetWindowLong ((HWND)handle, GWL_STYLE,
		(GetWindowLong((HWND)handle, GWL_STYLE) &~ WS_POPUP) | WS_CHILD);
	SetParent((HWND)handle, HWND(parent));
#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
#ifdef linux
	XReparentWindow(dp, handle, parent, 0, 0);
#endif
}
<<<<<<< HEAD

// add control

void CWin :: addControl(CCtrl* co)
{
	co->owner = this;
	controls->add(co);
}

#endif
=======

// add control

void CWin :: addControl(CCtrl* co)
{
	co->owner = this;
	controls->add(co);
}

#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
