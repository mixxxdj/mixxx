/***************************************************************************
                          windowkaiser.cpp  -  description
                             -------------------
    begin                : Sat Feb 8 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "windowkaiser.h"
#include "mathstuff.h"

WindowKaiser::WindowKaiser(int _size, CSAMPLE beta)
{
    size = _size;

    int m = size-1;
    afactor = 0.;

    window = new CSAMPLE[size];

    CSAMPLE t = besseli(beta);
    for (int k=0; k<size; k++)
    {
        window[k] = besseli(2.f*beta/m*sqrt((float)(k*(m-k))))/t;
        afactor += window[k];
    }

    afactor = 2.f/afactor;
}

WindowKaiser::~WindowKaiser()
{
    delete [] window;
}

int WindowKaiser::getSize()
{
    return size;
}

CSAMPLE * WindowKaiser::getWindowPtr()
{
    return window;
}

CSAMPLE WindowKaiser::getAFactor()
{
    return afactor;
}
