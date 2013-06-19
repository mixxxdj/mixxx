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

#ifndef CircularPeaksToPCP_hxx
#define CircularPeaksToPCP_hxx
#include <list>
#include <vector>
#include <cmath>

#ifdef __WINDOWS__
#define _USE_MATH_DEFINES
#include <math.h>
#endif

namespace Simac
{

/**
 * This processing constructs a PCP doing the weighted histogram
 * of the energy of a set of circular chromatic peaks.
 * It sums the energy of each peak to the corresponding pitch.
 * If windowing is activated by calling activateWindowing(),
 * then peaks on the center of the pitch get more relevance
 * while peaks in between the center of two pitch are attenuated.
 */
class CircularPeaksToPCP
{
public:
	typedef std::vector<std::pair<double, double> > PeakList;
	typedef std::vector<double> PCP;
private:
	PCP _output;
	bool _windowingActivated;
	static const unsigned nSemitones=12;
public:
	CircularPeaksToPCP()
	{
		_output.resize(nSemitones);
		_windowingActivated=false;
	}
	~CircularPeaksToPCP()
	{
	}
	void activateWindowing()
	{
		_windowingActivated=true;
	}
	static double windowedValue(double position, double value)
	{
		return value* (0.54 - 0.46*std::cos(2*M_PI*(position+.5)));
	}
	void doIt(const PeakList & peaks)
	{
		for (unsigned i=0; i<nSemitones; i++)
			_output[i]=0;
		const unsigned nPeaks=peaks.size();
		for (unsigned i=0; i<nPeaks; i++)
		{
			int quantizedSemitone = int(peaks[i].first + .5);
			unsigned semitone = (quantizedSemitone+nSemitones)%nSemitones;
			if (_windowingActivated)
				_output[semitone] += windowedValue(peaks[i].first,peaks[i].second);
			else
				_output[semitone] += peaks[i].second;
		}
	}

	const PCP & output() const
	{
		return _output;
	}
};

} // namespace Simac

#endif// CircularPeaksToPCP_hxx

