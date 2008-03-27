/***************************************************************************
                          enginefilterrbj.cpp  -  description
                             -------------------
    begin                : Wed Apr 3 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "enginefilterrbj.h"

EngineFilterRBJ::EngineFilterRBJ()
{
    // Reset filter coeffs
    b0a0=b1a0=b2a0=a1a0=a2a0=0.;

    // Reset in/out history
    ou1l=ou2l=in1l=in2l=ou1r=ou2r=in1r=in2r=0.0f;

    // Allocate buffer
    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterRBJ::~EngineFilterRBJ()
{
    delete [] buffer;
}

CSAMPLE * EngineFilterRBJ::process(const CSAMPLE * source, const int buf_size)
{
    for (int i=0; i<buf_size; i+=2)
    {
        // Filter left and right channel
        buffer[i]   = b0a0*source[i]   + b1a0*in1l + b2a0*in2l - a1a0*ou1l - a2a0*ou2l;
        buffer[i+1] = b0a0*source[i+1] + b1a0*in1r + b2a0*in2r - a1a0*ou1r - a2a0*ou2r;

        // Push in/out buffers
        in2l=in1l;
        in1l=source[i];
        ou2l=ou1l;
        ou1l=buffer[i];
        in2r=in1r;
        in1r=source[i+1];
        ou2r=ou1r;
        ou1r=buffer[i+1];
    }

    return buffer;
}

void EngineFilterRBJ::calc_filter_coeffs(int const type, double const frequency, double const sample_rate,
                                         double const q, double const db_gain, bool q_is_bandwidth)
{
    // temp pi
    double const temp_pi=3.1415926535897932384626433832795;

    // temp coef vars
    double alpha,a0,a1,a2,b0,b1,b2;

    // peaking, lowshelf and hishelf
    if(type>=6)
    {
        double const A      = pow(10.0,(db_gain/40.0));
        double const omega  = 2.0*temp_pi*frequency/sample_rate;
        double const tsin   = sin(omega);
        double const tcos   = cos(omega);

        if(q_is_bandwidth)
            alpha=tsin *sinh(log (2.0)/2.0 * q * omega/tsin);
        else
            alpha=tsin/(2.0*q);

        double const beta = sqrt(A)/q;

        // peaking
        if(type==6)
        {
            b0=float (1.0+alpha*A);
            b1=float (-2.0*tcos);
            b2=float (1.0-alpha*A);
            a0=float (1.0+alpha/A);
            a1=float (-2.0*tcos);
            a2=float (1.0-alpha/A);
        }

        // lowshelf
        if(type==7)
        {
            b0=float (A*((A+1.0)-(A-1.0)*tcos+beta*tsin));
            b1=float (2.0*A*((A-1.0)-(A+1.0)*tcos));
            b2=float (A*((A+1.0)-(A-1.0)*tcos-beta*tsin));
            a0=float ((A+1.0)+(A-1.0)*tcos+beta*tsin);
            a1=float (-2.0*((A-1.0)+(A+1.0)*tcos));
            a2=float ((A+1.0)+(A-1.0)*tcos-beta*tsin);
        }

        // hishelf
        if(type==8)
        {
            b0=float (A*((A+1.0)+(A-1.0)*tcos+beta*tsin));
            b1=float (-2.0*A*((A-1.0)+(A+1.0)*tcos));
            b2=float (A*((A+1.0)+(A-1.0)*tcos-beta*tsin));
            a0=float ((A+1.0)-(A-1.0)*tcos+beta*tsin);
            a1=float (2.0*((A-1.0)-(A+1.0)*tcos));
            a2=float ((A+1.0)-(A-1.0)*tcos-beta*tsin);
        }
    }
    else
    {
        // other filters
        double const omega      =       2.0*temp_pi*frequency/sample_rate;
        double const tsin       =       sin(omega);
        double const tcos       =       cos(omega);

        if(q_is_bandwidth)
            alpha=tsin *sinh(log (2.0)/2.0 * q * omega/tsin);
        else
            alpha=tsin/(2.0*q);

        // lowpass
        if(type==0)
        {
            b0=(1.0-tcos)/2.0;
            b1=1.0-tcos;
            b2=(1.0-tcos)/2.0;
            a0=1.0+alpha;
            a1=-2.0*tcos;
            a2=1.0-alpha;
        }

        // hipass
        if(type==1)
        {
            b0=(1.0+tcos)/2.0;
            b1=-(1.0+tcos);
            b2=(1.0+tcos)/2.0;
            a0=1.0+ alpha;
            a1=-2.0*tcos;
            a2=1.0-alpha;
        }

        // bandpass csg
        if(type==2)
        {
            b0=tsin/2.0;
            b1=0.0;
            b2=-tsin/2;
            a0=1.0+alpha;
            a1=-2.0*tcos;
            a2=1.0-alpha;
        }

        // bandpass czpg
        if(type==3)
        {
            b0=alpha;
            b1=0.0;
            b2=-alpha;
            a0=1.0+alpha;
            a1=-2.0*tcos;
            a2=1.0-alpha;
        }

        // notch
        if(type==4)
        {
            b0=1.0;
            b1=-2.0*tcos;
            b2=1.0;
            a0=1.0+alpha;
            a1=-2.0*tcos;
            a2=1.0-alpha;
        }

        // allpass
        if(type==5)
        {
            b0=1.0-alpha;
            b1=-2.0*tcos;
            b2=1.0+alpha;
            a0=1.0+alpha;
            a1=-2.0*tcos;
            a2=1.0-alpha;
        }
    }

    // set filter coeffs
    b0a0=float (b0/a0);
    b1a0=float (b1/a0);
    b2a0=float (b2/a0);
    a1a0=float (a1/a0);
    a2a0=float (a2/a0);

    //qDebug() << "coeff: " << b0a0 << ", " << b1a0 << ", " << b2a0 << ", " << a1a0 << ", " << a2a0;
}

