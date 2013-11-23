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
#include <cmath>
#include "ConstantQFolder.hxx"

namespace Simac
{

/**
 * returns the absolute value of complex number real + i*imag
 */
static double complexModule(const double & real, const double & imag)
{
	return std::sqrt(real*real + imag*imag);
}

ConstantQFolder::ConstantQFolder(unsigned nConstantQBins, int binsPerOctave) 
	: _binsPerOctave(binsPerOctave)
	, _nConstantQBins(nConstantQBins)
{
	_chromadata.resize(_binsPerOctave);
}

ConstantQFolder::~ConstantQFolder()
{
}

void ConstantQFolder::doIt(const std::vector<double> & constantQData)
{
	//initialise _chromadata to 0
	for (unsigned i=0; i<_binsPerOctave; i++) _chromadata[i]=0;

	// add each octave of cq data into chromagram
	const unsigned nOctaves = (int)floor(double(_nConstantQBins/_binsPerOctave))-1;
	unsigned constantQBin = 0;
	for (unsigned octave=0; octave<=nOctaves; octave++) {
		for (unsigned i=0; i<_binsPerOctave; i++) {
			const double & real = constantQData[constantQBin++];
			const double & imag = constantQData[constantQBin++];
			_chromadata[i] += complexModule(real, imag);
		}
	}
}

}

