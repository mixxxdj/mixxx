/***************************************************************************
                          enginespectralback.h  -  description
                             -------------------
    begin                : Sun Aug 5 2001
    copyright            : (C) 2001 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/


#ifndef ENGINESPECTRALBACK_H
#define ENGINESPECTRALBACK_H

#include "engineobject.h"
#include <math.h>
#include <sfftw.h>
#include <srfftw.h>

/**
  * Perfoms an inverse FFT on data in polar coordinates.
  *
  *@author Tue Haste Andersen
  */

class EngineSpectralBack : public EngineObject
{
public:
    /** Constructor. Length determines the length of the FFT */
    EngineSpectralBack(int Length, CSAMPLE *_window);
    /** Destructor */
    ~EngineSpectralBack();
    /** Performs the IFFT */
    CSAMPLE *process(CSAMPLE *p, const int buf_len);
protected:
    /** Internal fftw plan */
    rfftw_plan   plan_backward;
    /** Internal fftw plans */
    fftw_real   *tmp, *samples;
    /** Length, double length, and half the length of the fft */
    int    l, l2, l_half;
    CSAMPLE *window;
};

#endif
