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

#include <iostream>
#include "ConstantQTransform.hxx"
#include "FourierTransform.hxx"
#include <cmath>

namespace Simac
{
//---------------------------------------------------------------------------
// nextpow2 returns the smallest integer n such that 2^n >= x.
static double nextpow2(double x)
{
	return ceil(std::log(x)/std::log(2.0));
}
//----------------------------------------------------------------------------
static double squaredModule(const double & xx, const double & yy)
{
	return xx*xx + yy*yy;
}
//----------------------------------------------------------------------------

ConstantQTransform::ConstantQTransform(unsigned iFS, double ifmin, double ifmax, unsigned binsPerOctave) 
	: _binsPerOctave(binsPerOctave)
{
	FS = iFS;
	fmin = ifmin;		// min freq
	fmax = ifmax;		// max freq

	Q = 1/(std::pow(2.,(1/(double)_binsPerOctave))-1);	// Work out Q value for filter bank
	K = (int) ceil(_binsPerOctave * std::log(fmax/fmin)/std::log(2.0));	// No. of constant Q bins

	// work out length of fft required for this constant Q filter bank
	mSpectrumSize = (int) std::pow(2., nextpow2(ceil(Q*FS/fmin)));

	// allocate memory for cqdata
	cqdata.resize(2*K);
}

ConstantQTransform::~ConstantQTransform()
{
}

void ConstantQTransform::sparsekernel(double thresh)
{
	//generates spectral kernel matrix (upside down?)
	// initialise temporal kernel with zeros, twice length to deal w. complex numbers
	double* hammingWindow = new double [2*mSpectrumSize];
	for (unsigned mm=0; mm<2*mSpectrumSize; mm++) {
		hammingWindow[mm] = 0;
	}
	// Here, fftleng*2 is a guess of the number of sparse cells in the matrix
	// The matrix K x mSpectrumSize but the non-zero cells are an antialiased
	// square root function. So mostly is a line, with some grey point.
	mSparseKernelIs.reserve(mSpectrumSize*2);
	mSparseKernelJs.reserve(mSpectrumSize*2);
	mSparseKernelRealValues.reserve(mSpectrumSize*2);
	mSparseKernelImagValues.reserve(mSpectrumSize*2);
	
	// for each bin value K, calculate temporal kernel, take its fft to
	//calculate the spectral kernel then threshold it to make it sparse and 
	//add it to the sparse kernels matrix
	double squareThreshold = thresh * thresh;
	FourierTransform fft(mSpectrumSize, 1, true);
	for (unsigned k=K; k--; ) {
		// Computing a hamming window
		const unsigned hammingLength = (int) ceil(Q * FS / (fmin * std::pow(2.,((double)(k))/(double)_binsPerOctave)));
		for (unsigned i=0; i<hammingLength; i++) {
			const double angle = 2*M_PI*Q*i/hammingLength;
			const double real = cos(angle);
			const double imag = sin(angle);
			const double absol = Hamming(hammingLength, i)/hammingLength;
			hammingWindow[2*i  ] = absol*real;
			hammingWindow[2*i+1] = absol*imag;
		}

		//do fft of hammingWindow
		fft.doIt(hammingWindow);
		const double * transformedHamming = &fft.spectrum()[0];
		// 2 steps because they are complex
		for (unsigned j=0; j<(2*mSpectrumSize); j+=2) {
			// perform thresholding
			const double squaredBin = squaredModule(transformedHamming[j], transformedHamming[j+1]);
			if (squaredBin <= squareThreshold) continue;
			// Insert non-zero position indexes, doubled because they are floats
			mSparseKernelIs.push_back(j);
			mSparseKernelJs.push_back(k*2);
			// take conjugate, normalise and add to array sparkernel
			mSparseKernelRealValues.push_back( transformedHamming[j  ]/mSpectrumSize);
			mSparseKernelImagValues.push_back(-transformedHamming[j+1]/mSpectrumSize);
		}
	}
	delete [] hammingWindow;
}

//-----------------------------------------------------------------------------
void ConstantQTransform::doIt(const std::vector<double> & fftdata)
{
	// Multiply by sparse kernels matrix and store in cqdata
	// N.B. complex data
	// rows = mSpectrumSize
	// columns = K
	double * constantQSpectrum = &cqdata[0];
	const double * fftSpectrum = &fftdata[0];
	for (unsigned row=0; row<2*K; row+=2) {
		constantQSpectrum[row  ] = 0;
		constantQSpectrum[row+1] = 0;
	}
	const unsigned *fftbin = &(mSparseKernelIs[0]);
	const unsigned *cqbin = &(mSparseKernelJs[0]);
	const double   *real = &(mSparseKernelRealValues[0]);
	const double   *imag = &(mSparseKernelImagValues[0]);
	const unsigned int nSparseCells = mSparseKernelRealValues.size();
	for (unsigned i = 0; i<nSparseCells; i++)
	{
		const unsigned row = cqbin[i];
		const unsigned col = fftbin[i];
		const double & r1 = real[i];
		const double & i1 = imag[i];
		const double & r2 = fftSpectrum[col];
		const double & i2 = fftSpectrum[col+1];
		// add the multiplication
		constantQSpectrum[row  ] += r1*r2 - i1*i2;
		constantQSpectrum[row+1] += r1*i2 + i1*r2;
	}
}

}


