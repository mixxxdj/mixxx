/*
	dsp/Sine.h

	Copyright 2003-14 Tim Goetze <tim@quitte.de>

	http://quitte.de/dsp/

	Direct form I recursive sin() generator.  Utilising doubles
	for stability.

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

#ifndef DSP_SINE_H
#define DSP_SINE_H

namespace DSP {

class Sine
{
	public:
		int z;
		double y[2];
		double b;

	public:
		Sine()
			{
				b = 0;
				y[0] = y[1] = 0;
				z = 0;
			}

		Sine (double f, double fs, double phase)
			{
				set_f (f, fs, phase);
			}

		Sine (double w, double phase = 0.)
			{
				set_f (w, phase);
			}

		inline void set_f (double f, double fs, double phase)
			{
				set_f (f*2*M_PI/fs, phase);
			}

		inline void set_f (double w, double phase)
			{
				b = 2*cos(w);
				y[0] = sin (phase - w);
				y[1] = sin (phase - 2*w);
				z = 0;
			}

		/* advance and return 1 sample */
		inline double get()
			{
				register double s = b*y[z];
				z ^= 1;
				s -= y[z];
				return y[z] = s;
			}

		double get_phase()
			{
				double x0 = y[z], x1 = b*y[z] - y[z^1];
				double phi = asin(x0);

				/* slope is falling: into the 2nd half. */
				return x1 < x0 ? M_PI - phi : phi;
			}
};

/* same as above but including a damping coefficient d */
class DampedSine
: public Sine
{
	public:
		double d;

	public:
		DampedSine()
			{ d = 1; }

		inline double get()
			{
				register double s = b * y[z];
				z ^= 1;
				s -= d * y[z];
				return y[z] = d * s;
			}
};

} /* namespace DSP */

#endif /* DSP_SINE_H */
