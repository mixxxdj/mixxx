/***************************************************************************
                          enginespectralfwd.cpp  -  description
                             -------------------
    begin                : Sun Aug 5 2001
    copyright            : (C) 2001 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/


#include "enginespectralfwd.h"
#include "windowkaiser.h"
#include "mathstuff.h"

/* -------- -----------------------------------------------------------------
   Purpose: Initializes EngineSpectralFwd object, for performing fft's using the
            FFTW library.
   Input:   power - if true calculates power spectrum when performing tick()
            phase - if true calculates phase spectrum when performing tick()
            size  - size of FFT to perform
   Output:  -
   -------- ----------------------------------------------------------------- */
EngineSpectralFwd::EngineSpectralFwd(bool Power, bool Phase, WindowKaiser *window)
{
    l  = window->getSize();
    l_half = l/2;
    wndNorm = window->getAFactor();

    power_calc = Power;
    phase_calc = Phase;
    
    // Create plans for use in FFT calculations.
#ifndef __KISSFFT__
    plan_forward  = rfftw_create_plan(l,FFTW_REAL_TO_COMPLEX,FFTW_ESTIMATE);
    
    // Allocate temporary buffer. Double size arrays of the input size is used,
    // because of the fft's
    tmp = new fftw_real[l];
    spectrum = new fftw_real[l]; // Add two cells to make calculations of freq/phase pairs easier
#endif
#ifdef __KISSFFT__
    kisscfg = kiss_fftr_alloc(l, 0, 0, 0);
    tmp = new kiss_fft_cpx[l/2];
    spectrum = new kiss_fft_scalar[l];
#endif    
}

/* -------- -----------------------------------------------------------------
   Purpose: Destroys ESpectral object.
   Input:   -
   Output:  -
   -------- ----------------------------------------------------------------- */
EngineSpectralFwd::~EngineSpectralFwd()
{
    // Destroy fft plans
#ifndef __KISSFFT__
    rfftw_destroy_plan(plan_forward);
#endif
#ifdef __KISSFFT__
//    free(kisscfg);
#endif

    // Deallocate temporary buffer
//     delete [] tmp;
//     delete [] spectrum;
}

/* -------- -----------------------------------------------------------------
   Purpose: Performs fft on data pointed to by p.
   Input:   p is expected to hold a pointer to an array of size l2.
   Output:  A pointer to an array containing the power spectrum. The first
            half contains the power, the second the phase (see FFTW
            documentation for exact bounderies). If neither power_calc or
            phase_calc is true, a pointer to tmp is returned instead, giving
            the complex result of the fft.
   -------- ----------------------------------------------------------------- */
void EngineSpectralFwd::process(const CSAMPLE *pIn, const CSAMPLE *, const int)
{
    // Perform FFT
#ifndef __KISSFFT__
    fftw_real *pInput = (fftw_real *)pIn;
    rfftw_one(plan_forward, pInput, tmp);

    if (power_calc)
    {
        // Calculate length and angle of each vector
        //spectrum[0]      = tmp[0]     *tmp[0];  // Length of element 0 ... this gives a wrong value?!
        spectrum[l_half] = tmp[l_half]*tmp[l_half]; // Nyquist freq.
        for (int i=0; i<l_half; ++i)
            spectrum[i]  = sqrt(tmp[i]*tmp[i] + tmp[l-(i+1)]*tmp[l-(i+1)]);
        
//         qDebug("spec[10]: %f", spectrum[10]);
    }

    if (phase_calc)
    {
        //spectrum[l+2-1] = mod2pi(atan(0/tmp[0])); // Angle of element 0
        //spectrum[l_half+1] = 0;     // Angle of nyquist element
        for (int i=1; i<l_half; ++i)
            spectrum[l-i] = arctan2(tmp[l-i],tmp[i]);
    }
#endif
#ifdef __KISSFFT__
    kiss_fft_scalar *pInput = (kiss_fft_scalar *)pIn;
    kiss_fftr(kisscfg, pInput, tmp);

    if (power_calc)
    {
        // Calculate length and angle of each vector
        //spectrum[l_half] = tmp[l_half]*tmp[l_half]; // Nyquist freq.
        for (int i=0; i<l_half; ++i)
            spectrum[i]  = sqrt(tmp[i].r*tmp[i].r + tmp[i].i*tmp[i].i);
        
//         qDebug("spec[10]: %f", spectrum[10]);
    }

    if (phase_calc)
    {
        //spectrum[l+2-1] = mod2pi(atan(0/tmp[0])); // Angle of element 0
        //spectrum[l_half+1] = 0;     // Angle of nyquist element
        for (int i=1; i<l_half; ++i)
            spectrum[l-i] = arctan2(tmp[i].i,tmp[i].r);
    }
#endif

}

CSAMPLE EngineSpectralFwd::getHFC()
{
    ASSERT(power_calc);

    // Calculate sum of power spectrum
    CSAMPLE hfc = 0;

    for (int i=0; i<l_half; ++i)
    {
        CSAMPLE fr = (CSAMPLE)i/(CSAMPLE)l;
        hfc += wndNorm*spectrum[i]*(fr*fr)/(0.5*0.5); //(l_half*l_half);
    }
//     qDebug("hfc %f",hfc);
    //hfc *= (two_pi/l_half)*wndNorm;
    return hfc;
}

/* -------- -----------------------------------------------------------------
   Purpose: Returns the power of the spectrum at a specific bin. Expects the
            fft to be calculated by a call to process() before this method is
            called.
   Input:   index of the bin.
   Output:  Power value.
   -------- ----------------------------------------------------------------- */
CSAMPLE EngineSpectralFwd::power(int index)
{
#ifndef __KISSFFT__
    if (index==0)
        return sign(tmp[0])*tmp[0];
    else
        return sqrt(tmp[index]*tmp[index] + tmp[l-index]*tmp[l-index]);
#endif
#ifdef __KISSFFT__
    if (index==0)
        return sign(tmp[0].r)*tmp[0].r;
    else
        return sqrt(tmp[index].r*tmp[index].r + tmp[index].i*tmp[index].i);
#endif
}

/* -------- -----------------------------------------------------------------
   Purpose: Returns the phase of the spectrum at a specific bin. Expects the
            fft to be calculated by a call to process() before this method is
            called.
   Input:   index of the bin.
   Output:  Phase value.
   -------- ----------------------------------------------------------------- */
CSAMPLE EngineSpectralFwd::phase(int index)
{
    if (index==1)
#ifndef __KISSFFT__
        return mod2pi(atan(0/tmp[0])); // Angle of element 0
#endif        
#ifdef __KISSFFT__
        return mod2pi(atan(0/tmp[0].r)); // Angle of element 0
#endif        
    else
    {
        CSAMPLE phase;
#ifndef __KISSFFT__
        phase = arctan2(tmp[l-index],tmp[index]);
#endif        
#ifdef __KISSFFT__
        phase = arctan2(tmp[index].i,tmp[index].r);
#endif        
        return phase;
    }
}
