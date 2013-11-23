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

#ifndef SemitoneCenterFinder_hxx
#define SemitoneCenterFinder_hxx
#include <list>
#include <vector>
#include <cmath>

namespace Simac
{


class SemitoneCenterFinder
{
public:
	typedef std::vector<std::pair<double, double> > PeakList;
private:
	unsigned _histogramSize;
	unsigned * _histogram;
	unsigned _binsPerSemitone;
public:
	SemitoneCenterFinder()
	{
		_binsPerSemitone = 3;
		_histogramSize = 30;
		_histogram = new unsigned[_histogramSize];
		for (unsigned int i=0; i<_histogramSize; i++)
			_histogram[i]=0;
	}
	~SemitoneCenterFinder()
	{
		delete [] _histogram;
	}
	void doIt(unsigned int nPeaks, const double * peakPositions, const double * peakValues)
	{
		for (unsigned int i=0; i<nPeaks; i++)
		{
			double semitonePosition = std::fmod(peakPositions[i],_binsPerSemitone);
			unsigned histogramBin=semitonePosition*_histogramSize/_binsPerSemitone+0.5;
			_histogram[histogramBin]++;
		}
	}
	double output()
	{
		unsigned maxPos=0;
		unsigned maxOcurrences=0;
		for (unsigned int i=0; i<_histogramSize; i++)
		{
			if (_histogram[i]<=maxOcurrences) continue;
			maxOcurrences=_histogram[i];
			maxPos=i;
		}
		return maxPos*_binsPerSemitone/float(_histogramSize);
	}
};

} // namespace Simac

#endif// SemitoneCenterFinder_hxx

