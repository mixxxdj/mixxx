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

/* -------- -------------------------------cff----------------------------------
   Purpose: Initializes EngineSpectralFwd object, for performing fft's using the
            FFTW library.
   Input:   power - if true calculates power spectrum when performing tick()
            phase - if true calculates phase spectrum when performing tick()
            size  - size of FFT to perform
   Output:  -
   -------- ----------------------------------------------------------------- */
EngineSpectralFwd::EngineSpectralFwd(bool Power, bool Phase, WindowKaiser * window)
{
    l  = window->getSize();
    l_half = l/2;
    wndNorm = window->getAFactor();

    power_calc = Power;
    phase_calc = Phase;

    // Create plans for use in FFT calculations.
    kisscfg = kiss_fft_alloc(l, 0, 0, 0);
    tmp = new kiss_fft_cpx[l/2+1];
    spectrum = new kiss_fft_scalar[l];
    spectrumOld = new kiss_fft_scalar[l];

    for (int i=0; i<l; ++i)
    {
        spectrum[i] = 0.;
        spectrumOld[i] = 0.;
    }
}

/* -------- -----------------------------------------------------------------
   Purpose: Destroys ESpectral object.
   Input:   -
   Output:  -
   -------- ----------------------------------------------------------------- */
EngineSpectralFwd::~EngineSpectralFwd()
{
    // Destroy fft plans
    free(kisscfg);

    // Deallocate temporary buffer
    delete [] tmp;
    delete [] spectrum;
    delete [] spectrumOld;
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
void EngineSpectralFwd::process(const CSAMPLE * pIn, const CSAMPLE *, const int iBufferSize)
{
    //kiss_fft_cpx complexInput[iBufferSize];
    kiss_fft_cpx * complexInput;
    complexInput = new kiss_fft_cpx[iBufferSize];

    for (int i = 0; i < iBufferSize; i++)
    {
        complexInput[i].r = pIn[i];
        complexInput[i].i = 0.0f;
    }

    // Perform FFT
    kiss_fft_scalar * pInput = (kiss_fft_scalar *)pIn;
    kiss_fft(kisscfg, complexInput, tmp);

    // Shift pointers (spectrum/spectrumOld)
    kiss_fft_scalar * temp = spectrum;
    spectrum = spectrumOld;
    spectrumOld = temp;

    if (power_calc)
    {
        // Calculate length and angle of each vector
        //spectrum[l_half] = tmp[l_half]*tmp[l_half]; // Nyquist freq.
        for (int i=0; i<l_half; ++i)
            spectrum[i]  = sqrt(tmp[i].r*tmp[i].r + tmp[i].i*tmp[i].i);

//         qDebug() << "spec[10]: " << spectrum[10];
    }

    if (phase_calc)
    {
        //spectrum[l+2-1] = mod2pi(atan(0/tmp[0])); // Angle of element 0
        //spectrum[l_half+1] = 0;     // Angle of nyquist element
        for (int i=1; i<l_half; ++i)
            spectrum[l-i] = arctan2(tmp[i].i,tmp[i].r);
    }
    delete [] complexInput;
}

CSAMPLE EngineSpectralFwd::getHFC()
{
    Q_ASSERT(power_calc);

    // Calculate sum of power spectrum
    CSAMPLE hfc = 0;

    for (int i=0; i<l_half; ++i)
    {
        CSAMPLE fr = (CSAMPLE)i/(CSAMPLE)l;
        hfc += wndNorm*spectrum[i]*(fr*fr)/(0.5*0.5); //(l_half*l_half);
    }
//     qDebug() << "hfc " << hfc;
    //hfc *= (two_pi/l_half)*wndNorm;
    return hfc;
}

CSAMPLE EngineSpectralFwd::getPSF()
{
    Q_ASSERT(power_calc);
    float w;
    CSAMPLE psf = 0.;
    for (int i=1; i<l_half; ++i)
    {
        w = kfEqualLoudness[(int)((float)(i*kiEqualLoudnessLen)/(float)l_half)];
        //psf += w * sqrtf(spectrum[i]);
        psf += w * (spectrum[i]-spectrumOld[i]);
    }
    if (l_half != 0)  //Safety first
        psf /= l_half;

    return psf;
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
    if (index==0)
        return sign(tmp[0].r)*tmp[0].r;
    else
        return sqrt(tmp[index].r*tmp[index].r + tmp[index].i*tmp[index].i);
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
        return mod2pi(atan(0/tmp[0].r)); // Angle of element 0
    else
    {
        CSAMPLE phase;
        phase = arctan2(tmp[index].i,tmp[index].r);
        return phase;
    }
}

