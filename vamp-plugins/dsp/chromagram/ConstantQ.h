/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
    This file 2005-2006 Christian Landone.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef CONSTANTQ_H
#define CONSTANTQ_H

#include <vector>
#include "maths/MathAliases.h"
#include "maths/MathUtilities.h"

struct CQConfig{
    unsigned int FS;   // samplerate
    double min;        // minimum frequency
    double max;        // maximum frequency
    unsigned int BPO;  // bins per octave
    double CQThresh;   // threshold
};

class ConstantQ {
	
//public functions incl. sparsekernel so can keep out of loop in main
public:
    void process( const double* FFTRe, const double* FFTIm,
                  double* CQRe, double* CQIm );

    ConstantQ( CQConfig Config );
    ~ConstantQ();

    double* process( const double* FFTData );

    void sparsekernel();

    double hamming(int len, int n) {
	double out = 0.54 - 0.46*cos(2*PI*n/len);
	return(out);
    }
	
    int getnumwin() { return m_numWin;}
    double getQ() { return m_dQ;}
    int getK() {return m_uK ;}
    int getfftlength() { return m_FFTLength;}
    int gethop() { return m_hop;}

private:
    void initialise( CQConfig Config );
    void deInitialise();
	
    double* m_CQdata;
    unsigned int m_FS;
    double m_FMin;
    double m_FMax;
    double m_dQ;
    double m_CQThresh;
    unsigned int m_numWin;
    unsigned int m_hop;
    unsigned int m_BPO;
    unsigned int m_FFTLength;
    unsigned int m_uK;

    struct SparseKernel {
        std::vector<unsigned> is;
        std::vector<unsigned> js;
        std::vector<double> imag;
        std::vector<double> real;
    };

    SparseKernel *m_sparseKernel;
};


#endif//CONSTANTQ_H

