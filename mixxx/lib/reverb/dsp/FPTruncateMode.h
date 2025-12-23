/*
	FPTruncateMode.h
	
	Copyright 2001-11 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Sets the FP rounding mode to 'truncate' in the constructor
	and loads the previous FP conrol word in the destructor. 

	By directly using the machine instruction to convert float to int
	we avoid the performance hit that loading the control word twice for
	every (int) cast causes on i386.

	On other architectures this is a no-op.
 
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


#ifndef _DSP_FP_TRUNCATE_MODE_H_
#define _DSP_FP_TRUNCATE_MODE_H_

#ifdef __i386__
	#define fstcw(i) \
		__asm__ __volatile__ ("fstcw %0" : "=m" (i))

	#define fldcw(i) \
		__asm__ __volatile__ ("fldcw %0" : : "m" (i))

	/* gcc chokes on __volatile__ sometimes. */
	#define fistp(f,i) \
		__asm__ ("fistpl %0" : "=m" (i) : "t" (f) : "st")
#else /* ! __i386__ */
	#define fstcw(i)
	#define fldcw(i)

	#define fistp(f,i) \
			i = (int) f
#endif

namespace DSP {

static inline int 
fast_trunc (float f)
{
	int i;
	fistp (f, i);
	return i;
}
	
class FPTruncateMode
{
	public:
		int cw0, cw1; /* fp control word */

		FPTruncateMode()
			{
				fstcw (cw0);
				cw1 = cw0 | 0xC00;
				fldcw (cw1);
			}

		~FPTruncateMode()
			{
				fldcw (cw0);
			}
};

} /* namespace DSP */

#endif /* _DSP_FP_TRUNCATE_MODE_H_ */
