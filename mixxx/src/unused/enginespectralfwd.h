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
#include "kiss_fft.h"

class WindowKaiser;

// Table for equal loudness contour used in PSF calculation
static const int kiEqualLoudnessLen = 15;

static const float kfEqualLoudness[kiEqualLoudnessLen] = {1.5000f, 0.9934f, 0.9501f, 0.8812f, 
                                                          0.7975f, 0.7218f, 0.6385f, 0.5637f, 
                                                          0.4976f, 0.4393f, 0.3878f, 0.3615f,
                                                          0.3405f, 0.3207f, 0.3020f};


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
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    /** Return the High Frequency Content (HFC) of the spectrum */
    CSAMPLE getHFC();
    /** Return the Perceptual Spectral Flux (PSF) of the spectrum */
    CSAMPLE getPSF();
    /** Calculates the magnitude of the FFT at the bin at index. This function
      * only returns a valid result if both Power and Phase was set to false
      * upon initialization. */
    CSAMPLE power(int index);
    /** Calculates the phase of the FFT at the bin at index. This function
         * only returns a valid result if both Power and Phase was set to false
         * upon initialization. */
    CSAMPLE phase(int index);

protected:
    /** Kiss config */
    kiss_fft_cfg kisscfg;
    /** Internal buffers */
    kiss_fft_scalar *spectrum, *spectrumOld;
    kiss_fft_cpx *tmp;
    /** Length of the fft, and half the length of the fft */
    int    l, l_half; //l2
    /** Internal variables to determine if power and phase should be calculated
         * upon a call to tick(). */
    bool   power_calc, phase_calc;
    CSAMPLE wndNorm;
};

#endif
