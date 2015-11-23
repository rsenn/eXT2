#include <stdlib.h>
#include <math.h>
#include "filter1.h"


#define P2F 6.283185f


void Lowpass1::init (float fsam, float f3db)
{
    float w, c, s, d;

    w = P2F * f3db / fsam;
    c = cosf (w);
    s = sinf (w);
    d = (c < 1e-3f) ? (-0.5f * c) : ((s - 1) / c);
    _a = 0.5f * (1 + d);
}


void Allpass1::init (float fsam, float fmid)
{
    float w, c, s;

    w = P2F * fmid / fsam;
    c = cosf (w);
    s = sinf (w);
    _d = (c < 1e-3f) ? (-0.5f * c) : ((s - 1) / c);
}


void Pcshelf1::init (float fsam, float fmid, float glf, float ghf)
{
    float w, c, s, g, v;

    w = P2F * fmid / fsam;
    c = cosf (w);
    s = sinf (w);
    g = -glf / ghf;   
    g = (g - 1) / (g + 1);
    v = s * sqrt (1 - g * g) - 1;
    _d1 = (fabs (c - g) < 1e-3f) ? 0 : ((v + c * g) / (c - g));
    _d2 = (fabs (c + g) < 1e-3f) ? 0 : ((v - c * g) / (c + g));
    _g = glf * (1 + _d2) / (1 + _d1);
}


