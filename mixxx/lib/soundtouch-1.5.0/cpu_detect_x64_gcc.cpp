////////////////////////////////////////////////////////////////////////////////
///
/// Generic version of the x64 CPU detect routine.
///
/// This file is for GNU & other non-Windows compilers, see 'cpu_detect_x64_win.cpp' 
/// for the Microsoft compiler version.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2009-06-04 22:22:00 -0400 (Sun, 04 June 2009) $
// File revision : $Revision: 1 $
//
// $Id: cpu_detect_x64_gcc.cpp 1 2009-06-04 22:22:00Z pegasus $
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

#include "cpu_detect.h"

//////////////////////////////////////////////////////////////////////////////
//
// processor instructions extension detection routines
//
//////////////////////////////////////////////////////////////////////////////

// Flag variable indicating whick ISA extensions are disabled (for debugging)
static uint _dwDisabledISA = 0x00;      // 0xffffffff; //<- use this to disable all extensions


// Disables given set of instruction extensions. See SUPPORT_... defines.
void disableExtensions(uint dwDisableMask)
{
    _dwDisabledISA = dwDisableMask;
}



/// Checks which instruction set extensions are supported by the CPU.
uint detectCPUextensions(void)
{
    uint res = 0;

    if (_dwDisabledISA == 0xffffffff) return 0;

	// cpu_detect_x86_gcc segfaults on "%edx", "%eax", "%ecx", "%esi" );
	// Since I don't know how to fix that and just need something working for Mixxx...

	// All 64-bit processors support MMX, SSE, and SSE2
	res = SUPPORT_MMX + SUPPORT_SSE + SUPPORT_SSE2;

#ifdef x86_64
	res += SUPPORT_3DNOW;
#endif
	
	return res & ~_dwDisabledISA;
}
