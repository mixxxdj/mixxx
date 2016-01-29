/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
*/

#ifndef FFT_H
#define FFT_H

class FFT  
{
public:
    FFT(unsigned int nsamples);
    ~FFT();

    void process(bool inverse,
                 const double *realIn, const double *imagIn,
                 double *realOut, double *imagOut);
    
private:
    unsigned int m_n;
    void *m_private;
};

class FFTReal
{
public:
    FFTReal(unsigned int nsamples);
    ~FFTReal();

    void process(bool inverse,
                 const double *realIn,
                 double *realOut, double *imagOut);

private:
    unsigned int m_n;
    void *m_private;
};    

#endif
