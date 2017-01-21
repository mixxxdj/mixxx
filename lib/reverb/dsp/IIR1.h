/*
	dsp/OnePole.h

	Copyright 2003-14 Tim Goetze <tim@quitte.de>

	http://quitte.de/dsp/

	first-order IIR hi- and lo-pass filters.

*/
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA or point your web browser to http://www.gnu.org.
*/

#ifndef IIR1_H
#define IIR1_H

namespace DSP {

template <class T>
class LP1
{
	public:
		T a0, b1, y1;

		LP1 (double d = 1.)
			{
				set (d);
				y1 = 0.;
			}

		sample_t last() {return y1;}
		inline void reset() { y1 = 0.; }

		inline void decay (T d)
			{
				a0 *= d;
				b1 = 1. - a0;
			}

		inline void set_f (T fc)
			{
				set (1 - exp(-2*M_PI*fc));
			}
		inline void set (T d)
			{
				a0 = d;
				b1 = 1 - d;
			}

		inline T process (T x) { return y1 = a0*x + b1*y1; }
};

template <class T>
class HP1
{
	public:
		T a0, a1, b1, x1, y1;

		HP1 (T d = 1.)
			{
				set (d);
				reset();
			}

		sample_t last() {return y1;}

		void set_f (T f)
			{
				set (exp (-2*M_PI*f));
			}

		inline void set (T d)
			{
				a0 = .5*(1. + d);
				a1 = -.5*(1. + d);
				b1 = d;
			}

		inline T process (T x)
			{
				y1 = a0*x + a1*x1 + b1*y1;
				x1 = x;
				return y1;
			}

		void identity()
			{
				a0=1;
				a1=b1=0;
			}

		void reset()
			{
				x1 = y1 = 0;
			}
};

} /* namespace DSP */

#endif /* IIR1 */
