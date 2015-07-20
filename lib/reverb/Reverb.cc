/*
	Reverb.cc

	Copyright 2002-13 Tim Goetze <tim@quitte.de>

	Port from LADSPA to Mixxx 2014 by Owen Williams <owilliams@mixxx.org>,
	Mostly just deleting excess code.

	http://quitte.de/dsp/

	Three reverb units: JVRev, Plate and PlateX2.

	The former is a rewrite of STK's JVRev, a traditional design.

	Original comment:

		This is based on some of the famous
		Stanford CCRMA reverbs (NRev, KipRev)
		all based on the Chowning/Moorer/
		Schroeder reverberators, which use
		networks of simple allpass and comb
		delay filters.

	The algorithm is mostly unchanged in this implementation; the delay
	line lengths have been fiddled with to make the stereo field more
	evenly weighted, denormal protection and a bandwidth control have been
	added as well.

	The latter two are based on the circuit discussed in Jon Dattorro's
	September 1997 JAES paper on effect design (part 1: reverb & filters).
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
#include "Reverb.h"

/* //////////////////////////////////////////////////////////////////////// */

void
PlateStub::init()
{
	f_lfo = -1;
	// TODO(owilliams): use actual sample rate.
	fs = 44100;

#	define L(i) ((int) (l[i] * fs))
	static float l[] = {
		0.004771345048889486, 0.0035953092974026408,
		0.01273478713752898, 0.0093074829474816042,
		0.022579886428547427, 0.030509727495715868,
		0.14962534861059779, 0.060481838647894894, 0.12499579987231611,
		0.14169550754342933, 0.089244313027116023, 0.10628003091293972
	};

	/* lh */
	input.lattice[0].init (L(0));
	input.lattice[1].init (L(1));

	/* rh */
	input.lattice[2].init (L(2));
	input.lattice[3].init (L(3));

	/* modulated, width about 12 samples @ 44.1 */
	tank.mlattice[0].init (L(4), (int) (0.000403221 * fs));
	tank.mlattice[1].init (L(5), (int) (0.000403221 * fs));

	/* lh */
	tank.delay[0].init (L(6));
	tank.lattice[0].init (L(7));
	tank.delay[1].init (L(8));

	/* rh */
	tank.delay[2].init (L(9));
	tank.lattice[1].init (L(10));
	tank.delay[3].init (L(11));
#	undef L

#	define T(i) ((int) (t[i] * fs))
	static float t[] = {
		0.0089378717113000241, 0.099929437854910791, 0.064278754074123853,
		0.067067638856221232, 0.066866032727394914, 0.006283391015086859,
		0.01186116057928161, 0.12187090487550822, 0.041262054366452743,
		0.089815530392123921, 0.070931756325392295, 0.011256342192802662
	};

	for (int i = 0; i < 12; ++i)
		tank.taps[i] = T(i);
#	undef T

	/* tuned for soft attack, ambience */
	indiff1 = .742;
	indiff2 = .712;

	dediff1 = .723;
	dediff2 = .729;
}

void
PlateStub::process (sample_t x, sample_t decay, sample_t * _xl, sample_t * _xr)
{
	x = input.bandwidth.process (x);

	/* lh */
	x = input.lattice[0].process (x, indiff1);
	x = input.lattice[1].process (x, indiff1);

	/* rh */
	x = input.lattice[2].process (x, indiff2);
	x = input.lattice[3].process (x, indiff2);

	/* summation point */
	register double xl = x + decay * tank.delay[3].get();
	register double xr = x + decay * tank.delay[1].get();

	/* lh */
	xl = tank.mlattice[0].process (xl, dediff1);
	xl = tank.delay[0].putget (xl);
	xl = tank.damping[0].process (xl);
	xl *= decay;
	xl = tank.lattice[0].process (xl, dediff2);
	tank.delay[1].put (xl);

	/* rh */
	xr = tank.mlattice[1].process (xr, dediff1);
	xr = tank.delay[2].putget (xr);
	xr = tank.damping[1].process (xr);
	xr *= decay;
	xr = tank.lattice[1].process (xr, dediff2);
	tank.delay[3].put (xr);

	/* gather output */
	xl  = .6 * tank.delay[2] [tank.taps[0]];
	xl += .6 * tank.delay[2] [tank.taps[1]];
	xl -= .6 * tank.lattice[1] [tank.taps[2]];
	xl += .6 * tank.delay[3] [tank.taps[3]];
	xl -= .6 * tank.delay[0] [tank.taps[4]];
	xl += .6 * tank.lattice[0] [tank.taps[5]];

	xr  = .6 * tank.delay[0] [tank.taps[6]];
	xr += .6 * tank.delay[0] [tank.taps[7]];
	xr -= .6 * tank.lattice[0] [tank.taps[8]];
	xr += .6 * tank.delay[1] [tank.taps[9]];
	xr -= .6 * tank.delay[2] [tank.taps[10]];
	xr += .6 * tank.lattice[1] [tank.taps[11]];

	*_xl = xl;
	*_xr = xr;
}
