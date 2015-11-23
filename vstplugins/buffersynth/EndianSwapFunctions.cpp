//	EndianSwapFunctions.cpp - Functions to swap endian-ness for cross-platform
//							  file stuff.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include "EndianSwapFunctions.h"

short ShortSwap(short s)
{
  unsigned char b1, b2;
  
  b1 = s & 255;
  b2 = (s >> 8) & 255;

  return (b1 << 8) + b2;
}

short ShortNoSwap(short s)
{
  return s;
}

int LongSwap(int i)
{
  unsigned char b1, b2, b3, b4;

  b1 = i & 255;
  b2 = (i >> 8) & 255;
  b3 = (i>>16) & 255;
  b4 = (i>>24) & 255;

  return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int LongNoSwap(int i)
{
  return i;
}

float FloatSwap(float f)
{
  union
  {
    float f;
    unsigned char b[4];
  } dat1, dat2;

  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return dat2.f;
}

float FloatNoSwap(float f)
{
  return f;
}

bool BigEndianSystem;  //you might want to extern this
void InitEndian()
{
  unsigned char SwapTest[2] = { 1, 0 };
  
  if(*(short *)SwapTest == 1)
  {
    //little endian
    BigEndianSystem = false;

    //set func pointers to correct funcs
    BigShort = ShortSwap;
    LittleShort = ShortNoSwap;
    BigLong = LongSwap;
    LittleLong = LongNoSwap;
    BigFloat = FloatSwap;
    LittleFloat = FloatNoSwap;
  }
  else
  {
    //big endian
    BigEndianSystem = true;

    BigShort = ShortNoSwap;
    LittleShort = ShortSwap;
    BigLong = LongNoSwap;
    LittleLong = LongSwap;
    BigFloat = FloatNoSwap;
    LittleFloat = FloatSwap;
  }
}
