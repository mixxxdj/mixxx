/*
 * Copyright (c) 2001-2006 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ConstantQTransform_hxx
#define ConstantQTransform_hxx

#include <vector>
#include <cmath>

#ifdef __WINDOWS__
#define _USE_MATH_DEFINES
#include <math.h>
#endif

namespace Simac
{

/**
 * ConstantQTransform extract a ConstantQ spectrum using Blankertz's paper algorithm.
 * This transformation is similar to the Fourier transform but it generates
 * bins which bins are proportional to their center frequency.
 * 
 * This implementation saves computational time by moving most computation to
 * configuration time and using a fft as starting point for the transform.
 *
 * \b Parameters:
 * - samplinRate: The sampling rate for the input audio
 * - minFrequency: The minimum frequency to be considered
 * - maxFrequency: The maximum frequency to be considered
 * - binsPerOctave: The number of bins required for each octave
 *
 * \b Configuration Outputs:
 * - Q: Quality factor for the bins
 * - K: Number of Constant Q spectrum bins
 * - spectrumSize: Number of required fft spectrum bins
 *
 * \b Inputs:
 * - spectrum: An interlaced complex spectrum (complex, so, size 2*fftLength)
 *
 * \b Outputs:
 * - constantQSpectrum: An interlaced complex spectrum (complex, so, size 2*K)
 */

class ConstantQTransform
{
public:
	typedef std::vector<double> ConstantQSpectrum;
	typedef std::vector<double> Spectrum;
private:
	ConstantQSpectrum cqdata;
	unsigned FS;
	double fmin;
	double fmax;
	double Q;
	unsigned _binsPerOctave;
	unsigned mSpectrumSize;
	unsigned K;

	// Sparse complex numbers matrix represented by the i and j indexes
	// for non null cells and the real and imaginary components of the cell
	std::vector<unsigned> mSparseKernelIs;
	std::vector<unsigned> mSparseKernelJs;
	std::vector<double> mSparseKernelImagValues;
	std::vector<double> mSparseKernelRealValues;
public:
	void doIt(const std::vector<double> & fftData);

//public functions incl. sparsekernel so can keep out of loop in main
public:
	ConstantQTransform(unsigned FS, double fmin, double fmax, unsigned binsPerOctave);
	void sparsekernel(double);
	~ConstantQTransform();
	// Results
	const ConstantQSpectrum & constantQSpectrum() const {return cqdata;}
	double getQ() const {return Q;}
	int getK() const {return K;}
	int getfftlength() const {return mSpectrumSize;}
private:
	double Hamming(int len, int n) {
		double out = 0.54 - 0.46*std::cos(2*M_PI*n/len);
		return(out);
	}
};

}

#endif//ConstantQTransform_hxx

