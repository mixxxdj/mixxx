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

#ifndef ConstantQFolder_hxx
#define ConstantQFolder_hxx
#include <vector>
namespace Simac
{

/**
 * The ConstantQFolder takes a ConstantQ transform and folds its bins
 * into a single Octave to create a Chromagram.
 * \b Parameters:
 * - nConstantQBins: The number of bins on the Constant Q transform.
 * - binsPerOctave: The number of bins that corresponds to each octave.
 * \b Inputs:
 * - A constant Q transform as a vector containing the real and imaginary pairs
 *   bins, so it is size nConstantQBins * 2
 * \b Outputs:
 * - A chromogram of binsPerOctave bins.
 * @todo Only nConstantQBins that are multiple of binsPerOctave have been tested.
 */
class ConstantQFolder
{
public:
	typedef std::vector<double> Chromagram;
private:
	Chromagram _chromadata;
	unsigned _binsPerOctave;
	unsigned _nConstantQBins;
public:
	ConstantQFolder(unsigned nConstantQBins, int binsPerOctave);
	~ConstantQFolder();
	void doIt(const std::vector<double> & constantQData);

	/**
	 * Returns a chromagram that is the constant q transform for a a vector containing the added complex module of
	 * the bins that correspond to octaves.
	 */
	const Chromagram & chromagram() const {return _chromadata;}
};

}

#endif//ConstantQFolder_hxx 

