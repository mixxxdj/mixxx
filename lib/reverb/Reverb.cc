/*
	Reverb.cc

	Copyright 2002-14 Tim Goetze <tim@quitte.de>

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

#include "basics.h"

#include "Reverb.h"
//#include "Descriptor.h"

#if 0
int JVRev_length[9] = { 4199, 4999, 5399, 5801, 1051, 337, 113, 573, 487 };

void
JVRev::init()
{
	double s = fs/44100.;

	for (int i = 0; i < 9; ++i)
	{
		int v = (int) (s * JVRev_length[i]);
		v |= 1;
		while (!DSP::isprime(v))
			v += 2;
		length[i] = v;
	}

	for (int i = 0; i < 4; ++i)
		comb[i].init (length[i]);

	for (int i = 0; i < 3; ++i)
		allpass[i].init (length[i+4]);

	left.init (length[7]);
	right.init (length[8]);

	/* such a simple number, yet I couldn't find a better one. */
	apc = .7;
}

void
JVRev::set_t60 (sample_t t)
{
	t60 = t;

	t = max(.00001, t);
	t = -3/(t*fs);

	for (int i=0; i<4; ++i)
		comb[i].c = pow (10, t*length[i]);
}

void
JVRev::activate()
{
	bandwidth.reset();
	tone.reset();

	for (int i=0; i<3; ++i)
		allpass[i].reset();

	for (int i=0; i<4; ++i)
		comb[i].reset();

	left.reset();
	right.reset();

	set_t60(getport(1));
	tone.set_f(1800*over_fs);
}

void
JVRev::cycle (uint frames)
{
	sample_t bw = .005 + .994*getport(0);
	bandwidth.set(exp(-M_PI*(1. - bw)));

	if (t60 != *ports[1])
		set_t60(getport(1));

	double wet = getport(2);
	wet = .38*wet*wet;
	double dry = 1 - wet;

	sample_t * s = ports[3];

	sample_t * dl = ports[4];
	sample_t * dr = ports[5];

	for (uint i = 0; i < frames; ++i)
	{
		sample_t x = s[i], a = x + normal;

		a = bandwidth.process(a);
		x *= dry;

		/* diffusors */
		a = allpass[0].process(a,-apc);
		a = allpass[1].process(a,-apc);
		a = allpass[2].process(a,-apc);

		/* tank */
		sample_t t = 0;
		a -= normal;

		for (int j=0; j<4; ++j)
			t += comb[j].process(a);

		t = tone.process(t);

		dl[i] = x + wet*left.putget(t);
		dr[i] = x + wet*right.putget(t);
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
JVRev::port_info [] =
{
	{ "bandwidth", INPUT | CONTROL, {DEFAULT_MID, 0, 1} },
	{ "t60 (s)", INPUT | CONTROL | GROUP, {DEFAULT_MID, 0, 5.6} },
	{ "blend", INPUT | CONTROL, {DEFAULT_LOW, 0, 1} },

	{ "in", INPUT | AUDIO },
	{ "out.l", OUTPUT | AUDIO },
	{ "out.r", OUTPUT | AUDIO }
};

template <> void
Descriptor<JVRev>::setup()
{
	Label = "JVRev";

	Name = CAPS "JVRev - Stanford-style reverb from STK";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-12";

	/* fill port info and vtable */
	autogen();
}
#endif

/* //////////////////////////////////////////////////////////////////////// */

void
PlateStub::init()
{
	f_lfo = -1;

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

/* //////////////////////////////////////////////////////////////////////// */
#if 0
void
Plate::cycle (uint frames)
{
	sample_t bw = .005 + .994*getport(0);
	input.bandwidth.set (exp (-M_PI * (1. - bw)));

	sample_t decay = .749*getport(1);

	double damp = exp (-M_PI * (.0005+.9995*getport(2)));
	tank.damping[0].set (damp);
	tank.damping[1].set (damp);

	sample_t blend = getport(3);
	blend = pow (blend, 1.6); /* linear is not a good choice for this pot */
	sample_t dry = 1 - blend;

	sample_t * s = ports[4];

	sample_t * dl = ports[5];
	sample_t * dr = ports[6];

	/* modulated lattice interpolation needs float truncation */
	DSP::FPTruncateMode _truncate;

	for (uint i = 0; i < frames; ++i)
	{
		normal = -normal;
		sample_t x = s[i] + normal;

		sample_t xl, xr;

		PlateStub::process (x, decay, &xl, &xr);

		x = dry * s[i];

		dl[i] = x + blend*xl;
		dr[i] = x + blend*xr;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Plate::port_info [] =
{
	{"bandwidth", INPUT | CONTROL, {DEFAULT_HIGH, 0, 1} /* .9995 */ },
	{"tail", INPUT | CONTROL | GROUP, {DEFAULT_MID, 0, 1} /* .5 */ },
	{"damping", INPUT | CONTROL, {DEFAULT_LOW, 0, 1} /* .0005 */ },
	{"blend", INPUT | CONTROL | GROUP, {DEFAULT_LOW, 0, 1} },

	{"in", INPUT | AUDIO},
	{"out.l", OUTPUT | AUDIO},
	{"out.r",	OUTPUT | AUDIO}
};

template <> void
Descriptor<Plate>::setup()
{
	Label = "Plate";

	Name = CAPS "Plate - Versatile plate reverb";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-11";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
PlateX2::cycle (uint frames)
{
	sample_t bw = .005 + .994*getport(0);
	input.bandwidth.set (exp (-M_PI * (1. - bw)));

	sample_t decay = .749*getport(1);

	double damp = exp (-M_PI * (.0005+.9995*getport(2)));
	tank.damping[0].set (damp);
	tank.damping[1].set (damp);

	sample_t blend = getport(3);
	blend = pow (blend, 1.53);
	sample_t dry = 1 - blend;

	sample_t * sl = ports[4];
	sample_t * sr = ports[5];
	sample_t * dl = ports[6];
	sample_t * dr = ports[7];

	/* the modulated lattices interpolate, which needs truncated float */
	DSP::FPTruncateMode _truncate;

	for (uint i = 0; i < frames; ++i)
	{
		normal = -normal;
		sample_t x = (sl[i] + sr[i] + normal) * .5;

		sample_t xl, xr;
		PlateStub::process (x, decay, &xl, &xr);

		dl[i] = blend*xl + dry*sl[i];
		dr[i] = blend*xr + dry*sr[i];
	}
}

/* //////////////////////////////////////////////////////////////////////// */
PortInfo
PlateX2::port_info [] =
{
	{"bandwidth", INPUT | CONTROL, {DEFAULT_HIGH, 0, 1} /* .9995 */ },
	{"tail", INPUT | CONTROL | GROUP, {DEFAULT_MID, 0, 1} /* .5 */ },
	{"damping", INPUT | CONTROL, {DEFAULT_LOW, 0, 1} /* .0005 */ },
	{"blend", INPUT | CONTROL | GROUP, {DEFAULT_LOW, 0, 1} },

	{"in.l", INPUT | AUDIO},
	{"in.r", INPUT | AUDIO},
	{"out.l", OUTPUT | AUDIO},
	{"out.r",	OUTPUT | AUDIO}
};

template <> void
Descriptor<PlateX2>::setup()
{
	Label = "PlateX2";

	Name = CAPS "PlateX2 - Versatile plate reverb, stereo inputs";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-11";

	/* fill port info and vtable */
	autogen();
}
#endif



// (timrae) we have our left / right samples interleaved in the same array, so use slightly modified version of PlateX2::cycle
void MixxxPlateX2::processBuffer(const sample_t* in, sample_t* out, const uint frames, const sample_t bandwidthParam,
								const sample_t decayParam, const sample_t dampingParam, const sample_t blendParam) {
	// set bandwidth
	input.bandwidth.set(exp(-M_PI * (1. - (.005 + .994*bandwidthParam))));
	// set decay
	sample_t decay = .749*decayParam;
	// set damping
	double damp = exp(-M_PI * (.0005+.9995*dampingParam));
	tank.damping[0].set(damp);
	tank.damping[1].set(damp);
	// set blend
	sample_t blend = pow(blendParam, 1.53);
	sample_t dry = 1 - blend;

	// the modulated lattices interpolate, which needs truncated float
	DSP::FPTruncateMode _truncate;

	// loop through the buffer, processing each sample
	// note (timrae):treat the effect as SEND type instead of INSERT type for smoother parameter changes
	for (uint i = 0; i + 1 < frames; i += 2) {
		sample_t mono_sample = blend*(in[i] + in[i + 1]) / 2;
		sample_t xl, xr;
		PlateStub::process(mono_sample, decay, &xl, &xr);
		out[i] = xl + dry*in[i];
		out[i + 1] = xr + dry*in[i + 1];
	}
}
