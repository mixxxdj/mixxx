////////////////////////////////////////////////////////////////////////////////
///
/// gcc version of the MMX optimized routines. All MMX optimized functions
/// have been gathered into this single source code file, regardless to their 
/// class or original source code file, in order to ease porting the library
/// to other compiler and processor platforms.
///
/// This file is to be compiled on any platform with the GNU C compiler.
/// Compiler. Please see 'mmx_win.cpp' for the x86 Windows version of this
/// file.
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
// $Id: mmx_gcc.cpp 857 2005-02-10 13:11:55Z tuehaste $
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

#include <stdexcept>
#include <string>
#include "cpu_detect.h"

#ifndef __GNUC__
#error "wrong platform - this source code file is for the GNU C compiler."
#endif

using namespace std;
using namespace soundtouch;


#ifdef ALLOW_MMX
// MMX routines available only with integer sample type    

//////////////////////////////////////////////////////////////////////////////
//
// implementation of MMX optimized functions of class 'TDStretch'
//
// NOTE: ebx in gcc 3.x is not preserved if -fPIC and -DPIC
//  gcc-3.4 correctly flags this error and wont let you continue.
//  gcc-2.95 preserves esi correctly
//
//////////////////////////////////////////////////////////////////////////////

#include "TDStretch.h"
#include <limits.h>

// these are declared in 'TDStretch.cpp'
extern int scanOffsets[4][24];

// Calculates cross correlation of two buffers
long TDStretchMMX::calcCrossCorrStereo(const short *pV1, const short *pV2) const
{
#ifdef __i386__
    int corr;
    uint local_overlapLength = overlapLength;
    uint local_overlapDividerBits = overlapDividerBits;

    asm volatile(
        // Calculate cross-correlation between the tempOffset and tmpbid_buffer.

        // Process 4 parallel batches of 2 * stereo samples each during one
        // round to improve CPU-level parallellization.

        // load address of sloped pV2 buffer to eax
        // load address of mixing point of the sample data buffer to edi
        // load counter to ecx = overlapLength / 8 - 1
        // empty the mm0

        // prepare to the first round by loading 
        // load mm1 = eax[0]
        // load mm2 = eax[1];

        "\n\tmovl        %1, %%eax"
        "\n\tmovl        %2, %%edi"

        "\n\tmovq        (%%eax), %%mm1"
        "\n\tmovl        %3, %%ecx"

        "\n\tmovq        8(%%eax), %%mm2"
        "\n\tshr         $3, %%ecx"

        "\n\tpxor        %%mm0, %%mm0"
        "\n\tsub         $1, %%ecx"

        "\n\tmovd        %4, %%mm5"

        "\n1:"
        // multiply-add mm1 = mm1 * edi[0]
        // multiply-add mm2 = mm2 * edi[1]
        //
        // add mm2 += mm1
        // mm2 >>= mm5 (=overlapDividerBits)
        // add mm0 += mm2
        //
        // load mm3 = eax[2]
        // multiply-add mm3 = mm3 * edi[2]
        //
        // load mm4 = eax[3]
        // multiply-add mm4 = mm4 * edi[3]
        //
        // add mm3 += mm4
        // mm3 >>= mm5 (=overlapDividerBits)
        // add mm0 += mm3
        //
        // add eax += 4
        // add edi += 4
        // load mm1 = eax[0] (~eax[4])
        // load mm2 = eax[1] (~eax[5])
        //
        // loop

        "\n\tpmaddwd     (%%edi), %%mm1"   // qword ptr [edi]
        "\n\tmovq        16(%%eax), %%mm3" // qword ptr [eax+16]

        "\n\tpmaddwd     8(%%edi), %%mm2"  // qword ptr [edi+8]
        "\n\tmovq        24(%%eax), %%mm4" // qword ptr [eax+24]

        "\n\tpmaddwd     16(%%edi), %%mm3" // qword ptr [edi+16]
        "\n\tpaddd       %%mm1, %%mm2"

        "\n\tpmaddwd     24(%%edi), %%mm4" // qword ptr [edi+24]
        "\n\tmovq        32(%%eax), %%mm1" // qword ptr [eax+32]

        "\n\tpsrad       %%mm5, %%mm2"
        "\n\tadd         $32, %%eax"

        "\n\tpaddd       %%mm4, %%mm3"
        "\n\tpaddd       %%mm2, %%mm0"

        "\n\tmovq        8(%%eax), %%mm2"  // qword ptr [eax+8]
        "\n\tpsrad       %%mm5, %%mm3"

        "\n\tadd         $32, %%edi"
        "\n\tpaddd       %%mm3, %%mm0"

        "\n\tdec         %%ecx"
        "\n\tjnz         1b"

        // Finalize the last partial loop:

        "\n\tmovq        16(%%eax), %%mm3" // qword ptr [eax+16]
        "\n\tpmaddwd     (%%edi), %%mm1"   // qword ptr [edi]

        "\n\tmovq        24(%%eax), %%mm4" // qword ptr [eax+24]
        "\n\tpmaddwd     8(%%edi), %%mm2"  // qword ptr [edi+8]

        "\n\tpmaddwd     16(%%edi), %%mm3" // qword ptr [edi+16]
        "\n\tpaddd       %%mm1, %%mm2"

        "\n\tpmaddwd     24(%%edi), %%mm4" // qword ptr [edi+24]
        "\n\tpsrad       %%mm5, %%mm2"

        "\n\tpaddd       %%mm4, %%mm3"
        "\n\tpaddd       %%mm2, %%mm0"

        "\n\tpsrad       %%mm5, %%mm3"
        "\n\tpaddd       %%mm3, %%mm0"

        // copy hi-dword of mm0 to lo-dword of mm1, then sum mmo+mm1
        // and finally store the result into the variable "corr"

        "\n\tmovq        %%mm0, %%mm1"
        "\n\tpsrlq       $32, %%mm1"
        "\n\tpaddd       %%mm1, %%mm0"
        "\n\tmovd        %%mm0, %0"
      : "=rm" (corr)
      : "rim" (pV1), "rim" (pV2), "rim" (local_overlapLength),
        "rim" (local_overlapDividerBits)
      : "%ecx", "%eax", "%edi"
    );
    return corr;
    
    // Note: Warning about the missing EMMS instruction is harmless
    // as it'll be called elsewhere.
#else
    throw runtime_error("MMX not supported");
#endif
}

void TDStretchMMX::clearCrossCorrState()
{
#ifdef __i386__
    asm volatile("EMMS");
#endif
}

// MMX-optimized version of the function overlapStereo
void TDStretchMMX::overlapStereo(short *output, const short *input) const
{
#ifdef __i386__
    short *local_midBuffer = pMidBuffer;
    uint local_overlapLength = overlapLength;
    uint local_overlapDividerBits = overlapDividerBits;

    asm volatile(
        "\n\t"
        // load sliding mixing value counter to mm6 and mm7
        // load counter value to ecx = overlapLength / 4
        // load divider-shifter value to esi
        // load mixing value adder to mm5
        // load address of midBuffer to eax
        // load address of inputBuffer added with ovlOffset to edi
        // load address of end of the outputBuffer to edx
        //
        // We need to preserve esi, since gcc uses it for the
        // stack frame.

        "movl        %0, %%eax\n\t"               // ecx = 0x0000 OVL_
        "movl        $0x0002fffe, %%edi\n\t"      // ecx = 0x0002 fffe

        "movl        %1, %%esi\n\t"
        "movd        %%eax, %%mm6\n\t"            // mm6 = 0x0000 0000 0000 OVL_

        "movl        %%eax, %%ecx\n\t"
        "sub         $1, %%eax\n\t"

        "punpckldq   %%mm6, %%mm6\n\t"            // mm6 = 0x0000 OVL_ 0000 OVL_

        "or          $0x00010000, %%eax\n\t"      // eax = 0x0001 overlapLength-1

        "movd        %%edi, %%mm5\n\t"            // mm5 = 0x0000 0000 0002 fffe
        "movd        %%eax, %%mm7\n\t"            // mm7 = 0x0000 0000 0001 01ff

        "movl        %3, %%edi\n\t"

        "movl        %4, %%eax\n\t"               // dword ptr local_midBuffer
        "punpckldq   %%mm5, %%mm5\n\t"            // mm5 = 0x0002 fffe 0002 fffe

        "shr         $2, %%ecx\n\t"               // ecx = overlapLength / 2
        "punpckldq   %%mm7, %%mm7\n\t"            // mm7 = 0x0001 01ff 0001 01ff

        "movl        %2, %%edx\n"

        "2:\n\t"
        // Process two parallel batches of 2+2 stereo samples during each round 
        // to improve CPU-level parallellization.
        //
        // Load [eax] into mm0 and mm1
        // Load [edi] into mm3
        // unpack words of mm0, mm1 and mm3 into mm0 and mm1
        // multiply-add mm0*mm6 and mm1*mm7, store results into mm0 and mm1
        // divide mm0 and mm1 by 512 (=right-shift by overlapDividerBits)
        // pack the result into mm0 and store into [edx]
        //
        // Load [eax+8] into mm2 and mm3
        // Load [edi+8] into mm4
        // unpack words of mm2, mm3 and mm4 into mm2 and mm3
        // multiply-add mm2*mm6 and mm3*mm7, store results into mm2 and mm3
        // divide mm2 and mm3 by 512 (=right-shift by overlapDividerBits)
        // pack the result into mm2 and store into [edx+8]

    
        "movq        (%%eax), %%mm0\n\t"             // mm0 = m1l m1r m0l m0r
        "add         $16, %%edx\n\t"

        "movq        (%%edi), %%mm3\n\t"             // mm3 = i1l i1r i0l i0r
        "movq        %%mm0, %%mm1\n\t"               // mm1 = m1l m1r m0l m0r

        "movq        8(%%eax), %%mm2\n\t"            // mm2 = m3l m3r m2l m2r
        "punpcklwd   %%mm3, %%mm0\n\t"               // mm0 = i0l m0l i0r m0r

        "movq        8(%%edi), %%mm4\n\t"            // mm4 = i3l i3r i2l i2r
        "punpckhwd   %%mm3, %%mm1\n\t"               // mm1 = i1l m1l i1r m1r

        "movq        %%mm2, %%mm3\n\t"               // mm3 = m3l m3r m2l m2r
        "punpcklwd   %%mm4, %%mm2\n\t"               // mm2 = i2l m2l i2r m2r

        "pmaddwd     %%mm6, %%mm0\n\t"               // mm0 = i0l*m63+m0l*m62 i0r*m61+m0r*m60
        "punpckhwd   %%mm4, %%mm3\n\t"               // mm3 = i3l m3l i3r m3r

        "movd        %%esi, %%mm4\n\t"               // mm4 = overlapDividerBits

        "pmaddwd     %%mm7, %%mm1\n\t"               // mm1 = i1l*m73+m1l*m72 i1r*m71+m1r*m70
        "paddw       %%mm5, %%mm6\n\t"

        "paddw       %%mm5, %%mm7\n\t"
        "psrad       %%mm4, %%mm0\n\t"               // mmo >>= overlapDividerBits

        "pmaddwd     %%mm6, %%mm2\n\t"               // mm2 = i2l*m63+m2l*m62 i2r*m61+m2r*m60
        "psrad       %%mm4, %%mm1\n\t"               // mm1 >>= overlapDividerBits

        "pmaddwd     %%mm7, %%mm3\n\t"               // mm3 = i3l*m73+m3l*m72 i3r*m71+m3r*m70
        "psrad       %%mm4, %%mm2\n\t"               // mm2 >>= overlapDividerBits

        "packssdw    %%mm1, %%mm0\n\t"               // mm0 = mm1h mm1l mm0h mm0l
        "psrad       %%mm4, %%mm3\n\t"               // mm3 >>= overlapDividerBits

        "add         $16, %%eax\n\t"
        "paddw       %%mm5, %%mm6\n\t"

        "packssdw    %%mm3, %%mm2\n\t"               // mm2 = mm2h mm2l mm3h mm3l
        "paddw       %%mm5, %%mm7\n\t"

        "movq        %%mm0, -16(%%edx)\n\t"
        "add         $16, %%edi\n\t"

        "movq        %%mm2, -8(%%edx)\n\t"
        "dec         %%ecx\n\t"

        "jnz         2b\n\t"

        "emms\n\t"

      :
      : "rim" (local_overlapLength),
        "rim" (local_overlapDividerBits),
        "rim" (output),
        "rim" (input),
        "rim" (local_midBuffer)
      /* input */
      : "%edi", "%ecx", "%edx", "%eax", "%esi" /* regs */
    );
#else
    throw runtime_error("MMX not supported");
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// implementation of MMX optimized functions of class 'FIRFilter'
//
//////////////////////////////////////////////////////////////////////////////

#include "FIRFilter.h"

FIRFilterMMX::FIRFilterMMX() : FIRFilter()
{
    filterCoeffsUnalign = NULL;
}


FIRFilterMMX::~FIRFilterMMX()
{
    delete[] filterCoeffsUnalign;
}


#if 1
// (overloaded) Calculates filter coefficients for MMX routine
void FIRFilterMMX::setCoefficients(const short *coeffs, uint newLength, uint uResultDivFactor)
{
#ifdef __i386__
    uint i;
    FIRFilter::setCoefficients(coeffs, newLength, uResultDivFactor);

    // Ensure that filter coeffs array is aligned to 16-byte boundary
    delete[] filterCoeffsUnalign;
    filterCoeffsUnalign = new short[2 * newLength + 8];
    filterCoeffsAlign = (short *)(((uint)filterCoeffsUnalign + 15) & -16);

    // rearrange the filter coefficients for mmx routines 
    for (i = 0;i < length; i += 4) 
    {
        filterCoeffsAlign[2 * i + 0] = coeffs[i + 0];
        filterCoeffsAlign[2 * i + 1] = coeffs[i + 2];
        filterCoeffsAlign[2 * i + 2] = coeffs[i + 0];
        filterCoeffsAlign[2 * i + 3] = coeffs[i + 2];

        filterCoeffsAlign[2 * i + 4] = coeffs[i + 1];
        filterCoeffsAlign[2 * i + 5] = coeffs[i + 3];
        filterCoeffsAlign[2 * i + 6] = coeffs[i + 1];
        filterCoeffsAlign[2 * i + 7] = coeffs[i + 3];
    }
#else
    throw runtime_error("MMX not supported");
#endif
}



// mmx-optimized version of the filter routine for stereo sound
uint FIRFilterMMX::evaluateFilterStereo(short *dest, const short *src, const uint numSamples) const
{
#ifdef __i386__
    // Create stack copies of the needed member variables for asm routines :
    uint local_length = length;
    uint local_lengthDiv8 = lengthDiv8;
    uint local_resultDivider = resultDivFactor;
    short *local_filterCoeffs = (short*)filterCoeffsAlign;
    short *local_src = (short *)src;

    asm volatile(
        "\n\t"
        // Load (num_samples-aa_filter_length)/2 to edi as a i
        // Load a pointer to samples to esi
        // Load a pointer to destination to edx

        "movl        %0, %%edi\n\t"
        "subl        %2, %%edi\n\t"
        "movl        %3, %%edx\n\t"
        "sar         $1, %%edi\n"

        // Load filter length/8 to ecx
        // Load pointer to samples from esi to ebx
        // Load counter from edi to ecx
        // Load [ebx] to mm3
        // Load pointer to filter coefficients to eax
        "3:\n\t"
        "movl        %1, %%esi\n\t"
        "pxor        %%mm0, %%mm0\n\t"

        "movl        %4, %%ecx\n\t"
        "pxor        %%mm7, %%mm7\n\t"

        "movq        (%%esi), %%mm1\n\t"            // mm1 = l1 r1 l0 r0
        "movl        %5, %%eax\n"
        "4:\n\t"

        "movq        8(%%esi), %%mm2\n\t"           // mm2 = l3 r3 l2 r2
        "movq        %%mm1, %%mm4\n\t"              // mm4 = l1 r1 l0 r0

        "movq        16(%%esi), %%mm3\n\t"          // mm3 = l5 r5 l4 r4
        "punpckhwd   %%mm2, %%mm1\n\t"              // mm1 = l3 l1 r3 r1

        "movq        %%mm2, %%mm6\n\t"              // mm6 = l3 r3 l2 r2
        "punpcklwd   %%mm2, %%mm4\n\t"              // mm4 = l2 l0 r2 r0

        "movq        (%%eax), %%mm2\n\t"            // mm2 = f2 f0 f2 f0
        "movq        %%mm1, %%mm5\n\t"              // mm5 = l3 l1 r3 r1

        "punpcklwd   %%mm3, %%mm6\n\t"              // mm6 = l4 l2 r4 r2
        "pmaddwd     %%mm2, %%mm4\n\t"              // mm4 = l2*f2+l0*f0 r2*f2+r0*f0

        "pmaddwd     %%mm2, %%mm5\n\t"              // mm5 = l3*f2+l1*f0 r3*f2+l1*f0
        "movq        8(%%eax), %%mm2\n\t"           // mm2 = f3 f1 f3 f1

        "paddd       %%mm4, %%mm0\n\t"              // mm0 += s02*f02
        "movq        %%mm3, %%mm4\n\t"              // mm4 = l1 r1 l0 r0

        "pmaddwd     %%mm2, %%mm1\n\t"              // mm1 = l3*f3+l1*f1 r3*f3+l1*f1
        "paddd       %%mm5, %%mm7\n\t"              // mm7 += s13*f02

        "pmaddwd     %%mm2, %%mm6\n\t"              // mm6 = l4*f3+l2*f1 r4*f3+f4*f1
        "movq        24(%%esi), %%mm2\n\t"          // mm2 = l3 r3 l2 r2

        "paddd       %%mm1, %%mm0\n\t"              // mm0 += s31*f31
        "movq        32(%%esi), %%mm1\n\t"          // mm1 = l5 r5 l4 r4

        "paddd       %%mm6, %%mm7\n\t"              // mm7 += s42*f31
        "punpckhwd   %%mm2, %%mm3\n\t"              // mm3 = l3 l1 r3 r1

        "movq        %%mm2, %%mm6\n\t"              // mm6 = l3 r3 l2 r2
        "punpcklwd   %%mm2, %%mm4\n\t"              // mm4 = l2 l0 r2 r0

        "movq        16(%%eax), %%mm2\n\t"          // mm2 = f2 f0 f2 f0
        "movq        %%mm3, %%mm5\n\t"              // mm5 = l3 l1 r3 r1

        "punpcklwd   %%mm1, %%mm6\n\t"              // mm6 = l4 l2 r4 r2
        "add         $32, %%eax\n\t"

        "pmaddwd     %%mm2, %%mm4\n\t"              // mm4 = l2*f2+l0*f0 r2*f2+r0*f0
        "add         $32, %%esi\n\t"

        "pmaddwd     %%mm2, %%mm5\n\t"              // mm5 = l3*f2+l1*f0 r3*f2+l1*f0
        "movq        -8(%%eax), %%mm2\n\t"          // mm2 = f3 f1 f3 f1

        "paddd       %%mm4, %%mm0\n\t"              // mm0 += s02*f02
        "pmaddwd     %%mm2, %%mm3\n\t"              // mm3 = l3*f3+l1*f1 r3*f3+l1*f1

        "paddd       %%mm5, %%mm7\n\t"              // mm7 += s13*f02
        "pmaddwd     %%mm2, %%mm6\n\t"              // mm6 = l4*f3+l2*f1 r4*f3+f4*f1

        "paddd       %%mm3, %%mm0\n\t"              // mm0 += s31*f31
        "paddd       %%mm6, %%mm7\n\t"              // mm7 += s42*f31

        "dec         %%ecx\n\t"
        "jnz         4b\n\t"

        // Divide mm0 and mm7 by 8192 (= right-shift by 13),
        // pack and store to [edx]
        "movd        %6, %%mm4\n\t"

        "psrad       %%mm4, %%mm0\n\t"              // divide the result

        "add         $8, %%edx\n\t"
        "psrad       %%mm4, %%mm7\n\t"              // divide the result

        "add         $8, %1\n\t"
        "packssdw    %%mm7, %%mm0\n\t"

        "movq        %%mm0, -8(%%edx)\n\t"
        "dec         %%edi\n\t"

        "jnz         3b\n\t"

        "emms\n\t"

      :
      : "rim" (numSamples),
        "rim" (local_src),
        "rim" (local_length),
        "rim" (dest),
        "rim" (local_lengthDiv8),
        "rim" (local_filterCoeffs),
        "rim" (local_resultDivider) /* input */
      : "%eax", "%ecx", "%edx", "%edi", "%esi" /* regs */
    );
    return (numSamples & 0xfffffffe) - local_length;
#else
    throw runtime_error("MMX not supported");
    return 0;
#endif
}
#endif

#endif // ALLOW_MMX
