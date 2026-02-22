/*
	Reverb.h

	Copyright 2002-13 Tim Goetze <tim@quitte.de>

	http://quitte.de/dsp/

	two reverb units: JVRev and Plate.

	the former is a rewrite of STK's JVRev, a traditional design.

	original comment:

		This is based on some of the famous
		Stanford CCRMA reverbs (NRev, KipRev)
		all based on the Chowning/Moorer/
		Schroeder reverberators, which use
		networks of simple allpass and comb
		delay filters.

	(STK is an effort of Gary Scavone).

	the algorithm is mostly unchanged in this implementation; the delay
	line lengths have been fiddled with to make the stereo field more
	evenly weighted, and denormal protection has been added.

	the Plate reverb is based on the circuit discussed in Jon Dattorro's
	september 1997 JAES paper on effect design (part 1: reverb & filters).
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

#ifndef REVERB_H
#define REVERB_H

#include <stdio.h>
#include <cmath> // for M_PI

#include "basics.h"
#include "dsp/Delay.h"
#include "dsp/IIR1.h"
#include "dsp/Sine.h"
#include "dsp/util.h"

/* both reverbs use this */
class Lattice
: public DSP::Delay
{
	public:
		sample_t process (sample_t x, sample_t d)
			{
				sample_t y = get();
				x -= d*y;
				put(x);
				return d*x + y;
			}
};

/* helper for JVRev */
#if 0
class JVComb
: public DSP::Delay
{
	public:
		float c;

		sample_t process (sample_t x)
			{
				x += c*get();
				put(x);
				return x;
			}
};

class JVRev
: public Plugin
{
	public:
		DSP::LP1<sample_t> bandwidth, tone;

		sample_t t60;

		int length[9];

		Lattice allpass[3];
		JVComb comb[4];

		DSP::Delay left, right;

		double apc;

		void cycle (uint frames);

		void set_t60 (sample_t t);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};
#endif
/* /////////////////////////////////////////////////////////////////////// */

class ModLattice
{
	public:
		float n0, width;

		DSP::Delay delay;
		DSP::Sine lfo;

		void init (int n, int w)
			{
				n0 = static_cast<float>(n);
				width = static_cast<float>(w);
				delay.init (n + w);
			}

		void reset()
			{
				delay.reset();
			}

		inline sample_t
		process (sample_t x, sample_t d)
			{
				sample_t y = delay.get_linear (n0 + width * static_cast<float>(lfo.get()));
				x += d * y;
				delay.put (x);
				return y - d * x; /* note sign */
			}
};

class PlateStub

{
	public:
		sample_t f_lfo;
		sample_t indiff1, indiff2, dediff1, dediff2;

		struct {
			DSP::LP1<sample_t> bandwidth;
			Lattice lattice[4];
		} input;

		struct {
			ModLattice mlattice[2];
			Lattice lattice[2];
			DSP::Delay delay[4];
			DSP::LP1<sample_t> damping[2];
			int taps[12];
		} tank;

	public:
		void init();
		void activate()
			{
				input.bandwidth.reset();

				for (int i = 0; i < 4; ++i)
				{
					input.lattice[i].reset();
					tank.delay[i].reset();
				}

				for (int i = 0; i < 2; ++i)
				{
					tank.mlattice[i].reset();
					tank.lattice[i].reset();
					tank.damping[i].reset();
				}

				tank.mlattice[0].lfo.set_f (1.2, fs, 0);
				tank.mlattice[1].lfo.set_f (1.2, fs, .5 * M_PI);
			}

		void process (sample_t x, sample_t decay,
					sample_t * xl, sample_t * xr);
	protected:
		float fs = 44100; // (timrae) define sample rate
};

/* /////////////////////////////////////////////////////////////////////// */
/// (timrae) Disable the default classes as we use our own interface for Mixxx
#if 0
class Plate
: public PlateStub
{
	public:
		void cycle (uint frames);

	public:
		static PortInfo port_info [];
};

/* /////////////////////////////////////////////////////////////////////// */

class PlateX2
: public PlateStub
{
	public:
		void cycle (uint frames);

	public:
		static PortInfo port_info [];
};
#endif

/// (timrae) Define our own interface instead of using the original LADSPA plugin interface
 class MixxxPlateX2 : public PlateStub {
    public:
        void processBuffer(const sample_t* in, sample_t* out, const uint frames,
                           const sample_t bandwidthParam,
                           const sample_t decayParam,
                           const sample_t dampingParam,
                           const sample_t currentSend,
                           const sample_t previousSend);

        void init(float sampleRate) {
            PlateStub::init();
            setSamplerate(sampleRate);
        }

        void setSamplerate(float sampleRate) {
            fs = sampleRate;
            activate();
        }
 };

#endif /* REVERB_H */
