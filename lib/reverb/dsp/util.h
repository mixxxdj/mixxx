/*
	dsp/util.h
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Common math utility functions.

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

#ifndef _DSP_UTIL_H_
#define _DSP_UTIL_H_

namespace DSP {

inline float pow2 (float x) { return x * x; }
inline float pow3 (float x) { return x * pow2(x); }
inline float pow4 (float x) { return pow2 (pow2(x)); }
inline float pow5 (float x) { return x * pow4(x); }
inline float pow6 (float x) { return pow3 (pow2(x)); }
inline float pow7 (float x) { return x * (pow6 (x)); }
inline float pow8 (float x) { return pow2 (pow4 (x)); }

inline float 
sgn (float x)
{
	union { float f; uint32 i; } u;
	u.f = x;
	u.i &= 0x80000000;
	u.i |= 0x3F800000;
	return u.f;
}

inline bool
isprime (int v)
{
	if (v <= 3)
		return true;
	
	if (!(v & 1))
		return false;

	// NOTE(rryan): float cast added for MSVC.
	for (int i = 3; i < (int) sqrt ((float)v) + 1; i += 2)
		if ((v % i) == 0)
			return false;

	return true;
}

} /* namespace DSP */

#endif /* _DSP_UTIL_H_ */
