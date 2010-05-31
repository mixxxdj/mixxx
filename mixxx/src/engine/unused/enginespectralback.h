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
#ifndef __MACX__
#include <sfftw.h>
#include <srfftw.h>
#endif
#ifdef __MACX__
#include <fftw.h>
#include <rfftw.h>
#endif

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
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
protected:
    /** Internal fftw plan */
    rfftw_plan   plan_backward;
    /** Internal fftw plan */
    fftw_real   *tmp;
    /** Length, double length, and half the length of the fft */
    int    l, l2, l_half;
    CSAMPLE *window;
};

#endif
