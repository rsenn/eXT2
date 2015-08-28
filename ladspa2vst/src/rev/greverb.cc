#include <math.h>
#include <string.h>
#include "greverb.h"


void Diffuser::init (unsigned long size, float c)
{
    _size = size;
    _data = new float [size];
    _c = c;
    reset ();
}


void Diffuser::reset (void)
{
    memset (_data, 0, _size * sizeof (float));
    _i = 0;
}


void Diffuser::fini (void)
{
    delete[] _data;
}


void QuadFDN::init (unsigned long size)
{
    _size = size;
    for (int j = 0; j < 4; j++)
    {
        _data [j] = new float [size]; 
        _g [j] = 0;
        _d [j] = 0; 
    }
    _c = 1;
    reset ();
}       


void QuadFDN::reset (void)
{
    for (int j = 0; j < 4; j++)
    {
        memset (_data [j], 0, _size * sizeof (float));
        _y [j] = 0;
    }  
    _i = 0;
}       


void QuadFDN::fini (void)
{
    for (int j = 0; j < 4; j++) delete[] _data [j]; 
}


void MTDelay::init (unsigned long size)
{
    _size = size;
    _data = new float [size];
    for (int j = 0; j < 4; j++) _d [j] = 0;  
    _c = 1;
    reset ();
}


void MTDelay::reset (void)
{
    memset (_data, 0, _size * sizeof (float));
    for (int j = 0; j < 4; j++) _y [j] = 0;
    _z = 0;
    _i = 0; 
}


void MTDelay::fini (void)
{
    delete[] _data;
}


Greverb::Greverb (unsigned long rate) :
    _rate (rate),
    _roomsize (0.0),
    _revbtime (0.0),
    _ipbandw (0.8),
    _damping (0.2),
    _refllev (0.3),
    _taillev (0.3)
{
    unsigned long n;

    n = (unsigned long)(rate * 0.015);
    _dif0.init (n, 0.450);
    _dif1.init (n, 0.450);
    _qfdn.init ((unsigned long)(rate * MAX_ROOMSIZE / 340));
    n = (unsigned long)(_qfdn._size * 0.450);
    _del0.init (n);
    _del1.init (n);
    n = (unsigned long)(rate * 0.124);
    _dif1L.init ((unsigned long)(n * 0.2137), 0.5);
    _dif2L.init ((unsigned long)(n * 0.3753), 0.5);
    _dif3L.init (n - _dif1L._size - _dif2L._size, 0.5);
    _dif1R.init ((unsigned long)(n * 0.1974), 0.5);
    _dif2R.init ((unsigned long)(n * 0.3526), 0.5);
    _dif3R.init (n - _dif1R._size - _dif2R._size, 0.5);

    set_ipbandw (0.8);
    set_damping (0.2);
    set_roomsize (50.0);
    set_revbtime (3.0);
}


Greverb::~Greverb (void)
{
    _dif0.fini ();  
    _dif1.fini ();  
    _qfdn.fini ();
    _del0.fini ();;  
    _del1.fini ();;  
    _dif1L.fini ();  
    _dif2L.fini ();  
    _dif3L.fini ();  
    _dif1R.fini ();  
    _dif2R.fini ();  
    _dif3R.fini ();  
}


void Greverb::reset (void)
{
    // Clear all delay lines and filter states.
    // Current parameters are preserved.
    _dif0.reset ();  
    _dif1.reset ();  
    _qfdn.reset ();
    _del0.reset ();  
    _del1.reset ();  
    _dif1L.reset ();  
    _dif2L.reset ();  
    _dif3L.reset ();  
    _dif1R.reset ();  
    _dif2R.reset ();  
    _dif3R.reset ();  
}


void Greverb::set_roomsize (float R)
{
    if (R > MAX_ROOMSIZE) R = MAX_ROOMSIZE;   
    if (R < MIN_ROOMSIZE) R = MIN_ROOMSIZE;   
    if (fabs (_roomsize - R) < 0.5) return;
    _roomsize = R; 
    _qfdn._d [0] = (unsigned long)(_rate * R / 340.0);
    _qfdn._d [1] = (unsigned long)(_qfdn._d [0] * 0.816490);
    _qfdn._d [2] = (unsigned long)(_qfdn._d [0] * 0.707100);
    _qfdn._d [3] = (unsigned long)(_qfdn._d [0] * 0.632450);

    _del0._d [0] = (unsigned long)(_qfdn._d [0] * 0.100);
    _del0._d [1] = (unsigned long)(_qfdn._d [0] * 0.164);
    _del0._d [2] = (unsigned long)(_qfdn._d [0] * 0.270);
    _del0._d [3] = (unsigned long)(_qfdn._d [0] * 0.443);

    _del1._d [0] = (unsigned long)(_qfdn._d [0] * 0.087);
    _del1._d [1] = (unsigned long)(_qfdn._d [0] * 0.149);
    _del1._d [2] = (unsigned long)(_qfdn._d [0] * 0.256);
    _del1._d [3] = (unsigned long)(_qfdn._d [0] * 0.440);
    set_params ();    
}


void Greverb::set_revbtime (float T)
{
    if (T > MAX_REVBTIME) T = MAX_REVBTIME;   
    if (T < MIN_REVBTIME) T = MIN_REVBTIME;   
    if (fabs (_revbtime - T) < 0.05) return;
    _revbtime = T; 
    set_params ();
}


void Greverb::set_ipbandw (float B)
{
    if (B < 0.1) B = 0.1;
    if (B > 1.0) B = 1.0;
    _del1._c = _del0._c = _ipbandw = B;
}


void Greverb::set_damping (float D)
{
    if (D < 0.0) D = 0.0;
    if (D > 0.9) D = 0.9;
    _damping = D;
    _qfdn._c = 1.0 - _damping;
}


void Greverb::set_params (void)
{
    double a;

    a = pow (0.001, 1.0 / (_rate * _revbtime));
    for (int j = 0; j < 4; j++)
    {
        _qfdn._g [j] = pow (a, (double)(_qfdn._d [j]));
    }        
}


void Greverb::process (unsigned long n, float *x0, float *x1, float *y0, float *y1)
{
    float z, z0, z1;

    while (n--)
    {
        _del0.process (_dif0.process (*x0 + 1e-20));
        _del1.process (_dif1.process (*x1 + 1e-20));
        _qfdn.process (_del0._y, _del1._y);
        z = _taillev * (_qfdn._y [0] + _qfdn._y [1] + _qfdn._y [2] + _qfdn._y [3]);     
        z0 = _refllev * (_del0._y [0] - _del0._y [1] + _del0._y [2] - _del0._y [3]);     
        z1 = _refllev * (_del1._y [0] - _del1._y [1] + _del1._y [2] - _del1._y [3]);     
        *y0++ = _dif3L.process (_dif2L.process (_dif1L.process (z + z0))) + _dryslev * *x0++;
        *y1++ = _dif3R.process (_dif2R.process (_dif1R.process (z + z1))) + _dryslev * *x1++;
    }
}
