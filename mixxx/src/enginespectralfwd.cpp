/***************************************************************************
                          enginespectralfwd.cpp  -  description
                             -------------------
    begin                : Sun Aug 5 2001
    copyright            : (C) 2001 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/


#include "enginespectralfwd.h"
#include "mathstuff.h"

/* -------- -----------------------------------------------------------------
   Purpose: Initializes EngineSpectralFwd object, for performing fft's using the
            FFTW library.
   Input:   power - if true calculates power spectrum when performing tick()
            phase - if true calculates phase spectrum when performing tick()
            size  - size of FFT to perform
   Output:  -
   -------- ----------------------------------------------------------------- */
EngineSpectralFwd::EngineSpectralFwd(bool Power, bool Phase, int Length)
{
    l  = Length;
    //l2 = Length*2;
    l_half = Length/2;

    power_calc = Power;
    phase_calc = Phase;

    // Create plans for use in FFT calculations.
    plan_forward  = rfftw_create_plan(l,FFTW_REAL_TO_COMPLEX,FFTW_ESTIMATE);

    // Allocate temporary buffer. Double size arrays of the input size is used,
    // because of the fft's
    tmp = new fftw_real[l];
    spectrum = new fftw_real[l]; // Add two cells to make calculations of freq/phase pairs easier
}

/* -------- -----------------------------------------------------------------
   Purpose: Destroys ESpectral object.
   Input:   -
   Output:  -
   -------- ----------------------------------------------------------------- */
EngineSpectralFwd::~EngineSpectralFwd()
{
    // Destroy fft plans
    rfftw_destroy_plan(plan_forward);

    // Deallocate temporary buffer
    delete [] tmp;
    delete [] spectrum;
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
CSAMPLE *EngineSpectralFwd::process(const CSAMPLE *p, const int)
{
    if (p>0)
    {
        fftw_real *s = (fftw_real *)p;

        // Perform FFT
        rfftw_one(plan_forward, s, tmp);

        fftw_real *r = tmp;

        if (power_calc)
        {
            // Calculate length and angle of each vector
            spectrum[0]      = tmp[0]     *tmp[0];  // Length of element 0
            spectrum[l_half] = tmp[l_half]*tmp[l_half]; // Nyquist freq.
            for (int i=1; i<l_half; ++i)
                spectrum[i]  = sqrt(tmp[i]*tmp[i] + tmp[l-i]*tmp[l-i]);

            r = spectrum;
        }

        if (phase_calc)
        {
            //spectrum[l+2-1] = mod2pi(atan(0/tmp[0])); // Angle of element 0
            //spectrum[l_half+1] = 0;     // Angle of nyquist element
            for (int i=1; i<l_half; ++i)
                spectrum[l-i] = arctan2(tmp[l-i],tmp[i]);
            r = spectrum;
        }
        return r;
    }
    else
        return tmp;
}

/* -------- -----------------------------------------------------------------
   Purpose: Returns the power of the spectrum at a specific bin. Expects the
            fft to be calculated by a call to tick() before this method is
            called.
   Input:   index of the bin.
   Output:  Power value.
   -------- ----------------------------------------------------------------- */
CSAMPLE EngineSpectralFwd::power(int index)
{
    if (index==0)
        return sign(tmp[0])*tmp[0];
    else
        return sqrt(tmp[index]*tmp[index] + tmp[l-index]*tmp[l-index]);
}

/* -------- -----------------------------------------------------------------
   Purpose: Returns the phase of the spectrum at a specific bin. Expects the
            fft to be calculated by a call to tick() before this method is
            called.
   Input:   index of the bin.
   Output:  Phase value.
   -------- ----------------------------------------------------------------- */
CSAMPLE EngineSpectralFwd::phase(int index)
{
    if (index==1)
        return mod2pi(atan(0/tmp[0])); // Angle of element 0
    else
    {
        CSAMPLE phase;
        phase = arctan2(tmp[l-index],tmp[index]);
        return phase;
    }
}
