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

#ifndef CircularPeakPicking_hxx
#define CircularPeakPicking_hxx
#include <list>
#include <vector>

namespace Simac
{

/**
 * This processing takes a vector of scalars magnitudes and returns
 * a list of peaks found considering that the first and last bins
 * are neighbors.
 * Peaks are detected when there is a bin that is greater that neighbor
 * at both sides.
 * Then peak position and value are interpolated using a quadratic function
 * that passes also by the two neighbors bins.
 *
 * The first bin is considered at offset and each bin increases binSize.
 * By default, binSize and offset are 1 and 0 so that the bin position
 * matches the resulting position.
 *
 * @author David Garcia
 */
class CircularPeakPicking
{
public:
	typedef std::vector<std::pair<double, double> > PeakList;
private:
	PeakList _output;
	unsigned int _maxSize;
	double _binSize;
	double _offset;
public:
	CircularPeakPicking(unsigned chromagramSize, double binSize=1.0, double offset=0.0)
		: _maxSize(chromagramSize), _binSize(binSize), _offset(offset)
	{
		_output.reserve(chromagramSize);
	}
	/**
	*  Find the maximum of an interpolated quadratic polynomial function
	*  giving the samples at three equidistant points at x=0, x=1 and x=2.
	*  You can use it for any equidistant samples just by adding x0 to
	*  the resulting xmax.
	* 
	*  @pre The function will fail when y0>=y1 or y2>=y1, as it is supposed to
	*  be applied after having located a peak in y1.
	*  @returns A pair containing xmax,ymax
	*/
	std::pair<double,double> interpolate(double y0, double y1, double y2)
	{
		// From the quadratic lagrange interpolation
		// y=   y0(x1-x)(x2-x)/(x1-x0)(x2-x0) +
		//    + y1(x-x0)(x2-x)/(x1-x0)(x2-x0) +
		//    + y1(x2-x)(x-x0)/(x2-x1)(x2-x0) +
		//    + y2(x-x1)(x-x0)/(x2-x1)(x2-x0)  =
		//
		// considering x0=0, x1=1 and x2=2
		//
		//  =   y0(x-1)(x-2)/2 +
		//    - y1(x)(x-2)/2 +
		//    - y1(x-2)(x)/2 +
		//    + y2(x-1)(x)/2  =
		//
		//  =   y0(x-1)(x-2)/2 +
		//    - y1(x-2)(x) +
		//    + y2(x-1)(x)/2 =
		//
		//  = y0/2 (x^2 - 3x + 2) - y1 (x^2-2x) + y2/2 (x^2-x)
		//  = (y0/2-y1+y2/2) x^2 + (-3*y0/2 + 2*y1 - y2/2) x + y0

		double a = y0/2 - y1 + y2/2;
		double b = y1 -y0 -a; // = -3*y0/2 + 2*y1 -y2/2;
		double c = y0;

		// From equating to zero the derivate of x*x*a + x*b + c	
		double xmax = -b/(a*2);
		// ymax = xmax*xmax*a + b*xmax + c =
		//      = a*b*b/(4*a*a) -b*b/(2*a) + c =
		//      = b*b/(4*a) -b*b/(2*a) + c =
		//      = -b*b/(4*a) + c
		double ymax = b*xmax/2 + y0;

		return std::make_pair(xmax, ymax);
	}
	void doIt(const std::vector<double> & chromagram)
	{
		_output.resize(0);
		unsigned i0=_maxSize-2;
		unsigned i1=_maxSize-1;
		for (unsigned i=0; i<_maxSize; i0=i1, i1=i, i++)
		{
			// not equal to support plain two bins peaks
			if (chromagram[i0] >  chromagram[i1]) continue;
			if (chromagram[i ] >= chromagram[i1]) continue;

			std::pair<double,double> interpoled=
				interpolate(chromagram[i0],chromagram[i1],chromagram[i]);
			// Adding the base bin
			interpoled.first+=i0;
			// Folding to [0,_maxSize) interval
			while (interpoled.first<0) interpoled.first +=_maxSize;
			while (interpoled.first>=_maxSize) interpoled.first -=_maxSize;
			// Scaling
			interpoled.first*=_binSize;
			// Shifting to the first bin position
			interpoled.first+=_offset;

			_output.push_back(interpoled);
		}
	}
	const PeakList & output() const
	{
		return _output;
	}
};

} // namespace Simac

#endif// CircularPeakPicking_hxx

