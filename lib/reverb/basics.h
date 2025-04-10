#pragma once

/*
	basics.h

	Copyright 2004-12 Tim Goetze <tim@quitte.de>

	http://quitte.de/dsp/

	Common constants, typedefs, utility functions
	and simplified LADSPA #defines.

	Some code removed by Owen Williams for port to Mixxx, mostly ladspa-specific
	defines and i386 customizations.

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

#include <cassert> // for assert
#include <cstdint> // for uint32_t

typedef float sample_t;
typedef uint32_t uint32;
typedef unsigned int uint;
typedef unsigned long ulong;

/* lovely algorithm from
  http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
*/
inline uint
next_power_of_2 (uint n)
{
	assert (n <= 0x40000000);

	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;

	return ++n;
}

