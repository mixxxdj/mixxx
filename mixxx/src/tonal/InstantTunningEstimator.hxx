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

#ifndef InstantTunningEstimator_hxx
#define InstantTunningEstimator_hxx
#include <list>
#include <vector>
#include <cmath>

namespace Simac
{

/**
 * This processing estimates the most probable tunning of a set of chromatic peaks.
 * It does it by mapping the chromatic peaks as fasors
 * in a semitone wheel and vectorially adding them.
 * The inertia sets how much of the previous computed
 * value it kept for the next execution.
 * The instantTunning output doesn't take into account the inertia.
 * @todo Explain which are the reference values the tunning is expressed relative to.
 */
class InstantTunningEstimator
{
public:
	typedef std::vector<std::pair<double, double> > PeakList;
private:
	double _fasorX;
	double _fasorY;
	double _instantX;
	double _instantY;
	double _inertia;
public:
	InstantTunningEstimator(double inertia=0.0)
		: _inertia(inertia)
	{
		_fasorX=1.0;
		_fasorY=0.0;
	}
	~InstantTunningEstimator()
	{
	}
	void setInertia(double inertia)
	{
		_inertia=inertia;
	}
	// TODO: This function is taken by S&R of the previous one, no test!!
	void doIt(const std::vector<std::pair<double, double> >& peaks)
	{
		_fasorX*=_inertia;
		_fasorY*=_inertia;
		_instantX=0;
		_instantY=0;
		for (unsigned int peak=0; peak<peaks.size(); peak++)
		{
			double radiantTunning=peaks[peak].first*2*M_PI;
			_instantX+=cos(radiantTunning)*peaks[peak].second;
			_instantY+=sin(radiantTunning)*peaks[peak].second;
		}
		_fasorX += _instantX;
		_fasorY += _instantY;
	}
	std::pair<double,double> output() const
	{
		double tunning=std::atan2(_fasorY,_fasorX)/2/M_PI;
		double strength=std::sqrt(_fasorY*_fasorY+_fasorX*_fasorX);
		return std::make_pair(tunning, strength);
	}
	std::pair<double,double> instantTunning() const
	{
		double tunning=std::atan2(_instantY,_instantX)/2/M_PI;
		double strength=std::sqrt(_instantY*_instantY+_instantX*_instantX);
		return std::make_pair(tunning, strength);
	}
};

} // namespace Simac

#endif// InstantTunningEstimator_hxx

