/*

lib.h
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
#ifndef __LIB__
#define __LIB__

#include <stdlib.h>

// min

int asMin(int a, int b)
{
	if (a < b)
		return a;
	else return b;
}

// max

int asMax(int a, int b)
{
	if (a > b)
		return a;
	else return b;
}

// point

struct CPoint
{
	int x, y;
};

CPoint asPoint(int x, int y) 
{
	CPoint result;
	result.x = x;
	result.y = y;
	return result;
}

// rect

struct CRect
{
	int left, top, right, bottom;
};

CRect asRect(int x1, int y1, int x2, int y2)
{
	CRect result;
	result.left = x1;
	result.top = y1;
	result.right = x2;
	result.bottom = y2;
	return result;
}

CRect sizeRect(CRect rc, int x1, int y1, int x2, int y2)
{
	rc.left += x1;
	rc.top += y1;
	rc.right += x2;
	rc.bottom += y2;
	return rc;
}

// str

class CStr
{
	protected:
		char* buffer;
	public:
		CStr() 
			{ buffer = strdup(""); }
    CStr(const CStr& src) { 
			buffer = strdup(src.buffer); }
    CStr(const char* src) 
			{ buffer = strdup(src); }
    ~CStr() 
			{ if(buffer) free(buffer); }
    int length()  
			{ return strlen(buffer); }
		char* asChar() 
			{ return buffer; }
    CStr& operator = (const CStr& src)
		{
			if (buffer) free(buffer);
			buffer = strdup(src.buffer);
			return *this;
		}

};

// int to str

CStr asStr(int val)
{
	char str[255];
	sprintf(str, "%d", val);
	return CStr(str);
}

// float to str

CStr asStr(float val)
{
	char str[255];
	sprintf(str, "%f", val);
	return CStr(str);
}

// list

struct CListItem
{
	void* data;
	CListItem* next;
};

class CList
{
	protected:
		CListItem* head, *last;
	public:
		CList();
		~CList();
		void add(void* data);
		void* next(void** data);
};

CList :: CList()
{
	head = 0;
	last = 0;
}

CList :: ~CList()
{
	CListItem* p;
	while (head)
	{
		p = head;
		head = head->next;	
		delete p;
	}	
}

void CList :: add(void* data)
{
	CListItem* p = 	new CListItem;
	p->data = data;
	p->next = 0;
	if (head == 0)
		head = p;
	else last->next = p;
	last = p;
}

void* CList :: next(void** data)
{
	void* result = 0;
	if (*data == 0)	
		*data = head;
	else *data = ((CListItem*)*data)->next;	
	if (*data) 
		result = ((CListItem*)*data)->data;
	return result;
}

#endif
=======
#ifndef __LIB__
#define __LIB__

#include <stdlib.h>

// min

int asMin(int a, int b)
{
	if (a < b)
		return a;
	else return b;
}

// max

int asMax(int a, int b)
{
	if (a > b)
		return a;
	else return b;
}

// point

struct CPoint
{
	int x, y;
};

CPoint asPoint(int x, int y) 
{
	CPoint result;
	result.x = x;
	result.y = y;
	return result;
}

// rect

struct CRect
{
	int left, top, right, bottom;
};

CRect asRect(int x1, int y1, int x2, int y2)
{
	CRect result;
	result.left = x1;
	result.top = y1;
	result.right = x2;
	result.bottom = y2;
	return result;
}

CRect sizeRect(CRect rc, int x1, int y1, int x2, int y2)
{
	rc.left += x1;
	rc.top += y1;
	rc.right += x2;
	rc.bottom += y2;
	return rc;
}

// str

class CStr
{
	protected:
		char* buffer;
	public:
		CStr() 
			{ buffer = strdup(""); }
    CStr(const CStr& src) { 
			buffer = strdup(src.buffer); }
    CStr(const char* src) 
			{ buffer = strdup(src); }
    ~CStr() 
			{ if(buffer) free(buffer); }
    int length()  
			{ return strlen(buffer); }
		char* asChar() 
			{ return buffer; }
    CStr& operator = (const CStr& src)
		{
			if (buffer) free(buffer);
			buffer = strdup(src.buffer);
			return *this;
		}

};

// int to str

CStr asStr(int val)
{
	char str[255];
	sprintf(str, "%d", val);
	return CStr(str);
}

// float to str

CStr asStr(float val)
{
	char str[255];
	sprintf(str, "%f", val);
	return CStr(str);
}

// list

struct CListItem
{
	void* data;
	CListItem* next;
};

class CList
{
	protected:
		CListItem* head, *last;
	public:
		CList();
		~CList();
		void add(void* data);
		void* next(void** data);
};

CList :: CList()
{
	head = 0;
	last = 0;
}

CList :: ~CList()
{
	CListItem* p;
	while (head)
	{
		p = head;
		head = head->next;	
		delete p;
	}	
}

void CList :: add(void* data)
{
	CListItem* p = 	new CListItem;
	p->data = data;
	p->next = 0;
	if (head == 0)
		head = p;
	else last->next = p;
	last = p;
}

void* CList :: next(void** data)
{
	void* result = 0;
	if (*data == 0)	
		*data = head;
	else *data = ((CListItem*)*data)->next;	
	if (*data) 
		result = ((CListItem*)*data)->data;
	return result;
}

#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
