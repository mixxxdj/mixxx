/***************************************************************************
                          enginespectralfwd.h  -  description
                             -------------------
    begin                : Sun Aug 5 2001
    copyright            : (C) 2001 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/


#ifndef ENGINESPECTRALFWD_H
#define ENGINESPECTRALFWD_H

#include "engineobject.h"
#include <math.h>
#include <sfftw.h>
#include <srfftw.h>

class WindowKaiser;

/**
  * Performs FFT on a buffer of samples. Upon instantiazion it is possible
  * to select if the output should contain polar or rectangular corrdinates,
  * or a mixture. If rectangular is selected, functions are provided to
  * calculate the phase and magnitude for a given bin after the transform
  * has been calculated by a tick() operation.
  *
  *@author Tue Haste Andersen
  */

class EngineSpectralFwd : public EngineObject
{
public:
    /** Instantiation, if Power is true, the magnitude is calculated, if Phase
         * is true the phase is calculated, otherwise if Power and Phase are false,
         * the rectangular corrdinates are calculated. The returned array is in
         * a format specified by the FFTW library. */
    EngineSpectralFwd(bool Power, bool Phase, WindowKaiser *window);
    /** Destructor */
    ~EngineSpectralFwd();
    void notify(double) {};
    /** Performs an fft opeation on the samples pointed to by p */
    CSAMPLE *process(const CSAMPLE *p, const int);
    /** Return the High Frequency Content (HFC) of the spectrum */
    CSAMPLE getHFC();
    /** Calculates the magnitude of the FFT at the bin at index. This function
      * only returns a valid result if both Power and Phase was set to false
      * upon initialization. */
    CSAMPLE power(int index);
    /** Calculates the phase of the FFT at the bin at index. This function
         * only returns a valid result if both Power and Phase was set to false
         * upon initialization. */
    CSAMPLE phase(int index);

protected:
    /** Internal fftw plan */
    rfftw_plan   plan_forward;
    /** Internal fftw plans */
    fftw_real   *tmp, *spectrum; // Temporary FFT buffers
    /** Length of the fft, and half the length of the fft */
    int    l, l_half; //l2
    /** Internal variables to determine if power and phase should be calculated
         * upon a call to tick(). */
    bool   power_calc, phase_calc;
    CSAMPLE wndNorm;
};

#endif
