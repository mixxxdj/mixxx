// ------------------------------------------------------------------------
//
//  Copyright (C) 2010-2011 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// ------------------------------------------------------------------------


#include <string.h>
#include <math.h>
#include "ebu_r128_proc.h"


float Ebu_r128_hist::_bin_power [100] = { 0.0f };
float Ebu_r128_proc::_chan_gain [5] = { 1.0f, 1.0f, 1.0f, 1.41f, 1.41f };


Ebu_r128_hist::Ebu_r128_hist (void)
{
    _histc = new int [751];
    initstat ();
    reset ();
}


Ebu_r128_hist::~Ebu_r128_hist (void)
{
    delete[] _histc;
}


void Ebu_r128_hist::reset (void)
{
    memset (_histc, 0, 751 * sizeof (float));
    _count = 0;
    _error = 0;
}


void Ebu_r128_hist::initstat (void)
{
    int i;

    if (_bin_power [0]) return;
    for (i = 0; i < 100; i++)
    {
	_bin_power [i] = powf (10.0f, i / 100.0f);
    }
}


void Ebu_r128_hist::addpoint (float v)
{
    int k;

    k = (int) floorf (10 * v + 700.5f);
    if (k < 0) return;
    if (k > 750)
    {
	k = 750;
	_error++;
    }
    _histc [k]++;
    _count++;
}


float Ebu_r128_hist::integrate (int i)
{
    int   j, k, n;
    float s;

    j = i % 100;
    n = 0;
    s = 0;
    while (i <= 750)
    {
	k = _histc [i++];
	n += k;
	s += k * _bin_power [j++];
	if (j == 100)
	{
	    j = 0;
	    s /= 10.0f;
	}
    }	
    return s / n;
}


void Ebu_r128_hist::calc_integ (float *vi, float *th)
{
    int   k;
    float s;

    if (_count < 50)
    {
        *vi = -200.0f;
	return;
    }
    s = integrate (0);
//  Original threshold was -8 dB below result of first integration
//    if (th) *th = 10 * log10f (s) - 8.0f;
//    k = (int)(floorf (100 * log10f (s) + 0.5f)) + 620;
//  Threshold redefined to -10 dB below result of first integration
    if (th) *th = 10 * log10f (s) - 10.0f;
    k = (int)(floorf (100 * log10f (s) + 0.5f)) + 600;
    if (k < 0) k = 0;
    s = integrate (k);
    *vi = 10 * log10f (s);
}


void Ebu_r128_hist::calc_range (float *v0, float *v1, float *th)
{
    int   i, j, k, n;
    float a, b, s;

    if (_count < 20)
    {
	*v0 = -200.0f;
	*v1 = -200.0f;
        return;
    }
    s = integrate (0);
    if (th) *th = 10 * log10f (s) - 20.0f;
    k = (int)(floorf (100 * log10f (s) + 0.5)) + 500;
    if (k < 0) k = 0;
    for (i = k, n = 0; i <= 750; i++) n += _histc [i]; 
    a = 0.10f * n;
    b = 0.95f * n;
    for (i =   k, s = 0; s < a; i++) s += _histc [i];
    for (j = 750, s = n; s > b; j--) s -= _histc [j];
    *v0 = (i - 701) / 10.0f;
    *v1 = (j - 699) / 10.0f;
}




Ebu_r128_proc::Ebu_r128_proc (void)
{
    reset ();
}


Ebu_r128_proc::~Ebu_r128_proc (void)
{
}


void Ebu_r128_proc::init (int nchan, float fsamp)
{
    _nchan = nchan;
    _fsamp = fsamp;
    _fragm = (int) fsamp / 20;
    detect_init (_fsamp);
    reset ();
}


void Ebu_r128_proc::reset (void)
{
    _integr = false;
    _frcnt = _fragm;
    _frpwr = 1e-30f;
    _wrind  = 0;
    _div1 = 0;
    _div2 = 0;
    _loudness_M = -200.0f;
    _loudness_S = -200.0f;
    memset (_power, 0, 64 * sizeof (float));
    integr_reset ();
    detect_reset ();
}


void Ebu_r128_proc::integr_reset (void)
{
    _hist_M.reset ();
    _hist_S.reset ();
    _maxloudn_M = -200.0f;
    _maxloudn_S = -200.0f;
    _integrated = -200.0f;
    _integ_thr  = -200.0f;
    _range_min  = -200.0f;
    _range_max  = -200.0f;
    _range_thr  = -200.0f;
    _div1 = _div2 = 0;
}


void Ebu_r128_proc::process (int nfram, float *input [])
{
    int  i, k;
    
    for (i = 0; i < _nchan; i++) _ipp [i] = input [i];
    while (nfram)
    {
	k = (_frcnt < nfram) ? _frcnt : nfram;
	_frpwr += detect_process (k);
        _frcnt -= k;
	if (_frcnt == 0)
	{
	    _power [_wrind++] = _frpwr / _fragm;
	    _frcnt = _fragm;
	    _frpwr = 1e-30f;
	    _wrind &= 63;
	    _loudness_M = addfrags (8);
	    _loudness_S = addfrags (60);
            if (_loudness_M > _maxloudn_M) _maxloudn_M = _loudness_M;
            if (_loudness_S > _maxloudn_S) _maxloudn_S = _loudness_S;
	    if (_integr)
	    {
	        if (++_div1 == 2)
  	        {
		    _hist_M.addpoint (_loudness_M);
		    _div1 = 0;
	        }
	        if (++_div2 == 10)
	        {
		    _hist_S.addpoint (_loudness_S);
		    _div2 = 0;
		    _hist_M.calc_integ (&_integrated, &_integ_thr);
		    _hist_S.calc_range (&_range_min, &_range_max, &_range_thr);
		}
	    }
	}
	for (i = 0; i < _nchan; i++) _ipp [i] += k;
	nfram -= k;
    }
}


float Ebu_r128_proc::addfrags (int nfrag)
{
    int    i, k;
    float  s;

    s = 0;
    k = (_wrind - nfrag) & 63;
    for (i = 0; i < nfrag; i++) s += _power [(i + k) & 63];
    return -0.6976f + 10 * log10f (s / nfrag);
}


void Ebu_r128_proc::detect_init (float fsamp)
{
    float a, b, c, d, r, u1, u2, w1, w2;

    r = 1 / tan (4712.3890f / fsamp);
    w1 = r / 1.12201f; 
    w2 = r * 1.12201f;
    u1 = u2 = 1.4085f + 210.0f / fsamp;
    a = u1 * w1;
    b = w1 * w1;
    c = u2 * w2;
    d = w2 * w2;
    r = 1 + a + b;
    _a0 = (1 + c + d) / r;
    _a1 = (2 - 2 * d) / r;
    _a2 = (1 - c + d) / r;
    _b1 = (2 - 2 * b) / r;
    _b2 = (1 - a + b) / r;
    r = 48.0f / fsamp;
    a = 4.9886075f * r;
    b = 6.2298014f * r * r;
    r = 1 + a + b;
    a *= 2 / r;
    b *= 4 / r;
    _c3 = a + b;
    _c4 = b;
    r = 1.004995f / r;
    _a0 *= r;
    _a1 *= r;
    _a2 *= r;
}


void Ebu_r128_proc::detect_reset (void)
{
    for (int i = 0; i < MAXCH; i++) _fst [i].reset ();
}


float Ebu_r128_proc::detect_process (int nfram)
{
    int   i, j;
    float si, sj;
    float x, y, z1, z2, z3, z4;
    float *p;
    Ebu_r128_fst *S;

    si = 0;
    for (i = 0, S = _fst; i < _nchan; i++, S++)
    {
	z1 = S->_z1;
	z2 = S->_z2;
	z3 = S->_z3;
	z4 = S->_z4;
	p = _ipp [i];
	sj = 0;
	for (j = 0; j < nfram; j++)
	{
	    x = p [j] - _b1 * z1 - _b2 * z2 + 1e-15f;
	    y = _a0 * x + _a1 * z1 + _a2 * z2 - _c3 * z3 - _c4 * z4;
	    z2 = z1;
	    z1 = x;
	    z4 += z3;
	    z3 += y;
	    sj += y * y;
	}
	if (_nchan == 1) si = 2 * sj;
	else si += _chan_gain [i] * sj;
	S->_z1 = z1;
	S->_z2 = z2;
	S->_z3 = z3;
	S->_z4 = z4;
    }
    return si;
}



