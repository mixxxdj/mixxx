////////////////////////////////////////////////////////////////////////////////
///
/// Common type definitions for SoundTouch audio processing library.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai @ iki.fi
/// SoundTouch WWW: http://www.iki.fi/oparviai/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2005-02-10 13:11:55 +0000 (Thu, 10 Feb 2005) $
// File revision : $Revision: 857 $
//
// $Id: STTypes.h 857 2005-02-10 13:11:55Z tuehaste $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#ifndef STTypes_H
#define STTypes_H

typedef unsigned int    uint;
typedef unsigned long   ulong;

#ifndef _WINDEF_
    // if these aren't defined already by Windows headers, define now

    typedef unsigned int    BOOL;

    #define FALSE   0
    #define TRUE    1

#endif  // _WINDEF_


namespace soundtouch
{
    /// Enable one of the following defines to choose either 16bit integer or
    /// 32bit float sample type. If you don't have opinion, using integer samples
    /// is generally faster.
    #define INTEGER_SAMPLES       //< 16bit integer samples
    //#define FLOAT_SAMPLES       //< 32bit float samples


    /// Define this to allow CPU-specific assembler optimizations. Notice that 
    /// having this enabled on non-x86 platforms doesn't matter; the compiler can 
    /// drop unsupported extensions on different platforms automatically. 
    /// However, if you're having difficulties getting the optimized routines 
    /// compiled with your compler (e.g. some gcc compiler versions may be picky), 
    /// you may wish to disable the optimizations to make the library compile.
    #define ALLOW_OPTIMIZATIONS     1


    #ifdef INTEGER_SAMPLES
        // 16bit integer sample type
        typedef short SAMPLETYPE;
        // data type for sample accumulation: Use 32bit integer to prevent overflows
        typedef long  LONG_SAMPLETYPE;

        #ifdef FLOAT_SAMPLES
            // check that only one sample type is defined
            #error "conflicting sample types defined"
        #endif // FLOAT_SAMPLES

        #ifdef ALLOW_OPTIMIZATIONS
            #if WIN32 || __i386__
                // Allow MMX optimizations
                #define ALLOW_MMX   1
            #endif
        #endif

    #else

        // floating point samples
        typedef float  SAMPLETYPE;
        // data type for sample accumulation: Use double to utilize full precision.
        typedef double LONG_SAMPLETYPE;

        #ifdef ALLOW_OPTIMIZATIONS
            #ifdef WIN32
                // Allow 3DNow! and SSE optimizations
                #define ALLOW_3DNOW     1
                #define ALLOW_SSE       1
            #endif // WIN32
        #endif

    #endif  // INTEGER_SAMPLES
};

#endif
