## Process this file with automake to create Makefile.in
##
## $Id: Makefile.am 230 2015-09-20 07:38:32Z oparviai $
##
## This file is part of SoundTouch, an audio processing library for pitch/time adjustments
## 
## SoundTouch is free software; you can redistribute it and/or modify it under the
## terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
## 
## SoundTouch is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
## A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the Free Software Foundation, Inc., 59 Temple
## Place - Suite 330, Boston, MA  02111-1307, USA


include $(top_srcdir)/config/am_include.mk


# set to something if you want other stuff to be included in the distribution tarball
EXTRA_DIST=SoundTouch.dsp SoundTouch.dsw SoundTouch.sln SoundTouch.vcproj

noinst_HEADERS=AAFilter.h cpu_detect.h cpu_detect_x86.cpp FIRFilter.h RateTransposer.h TDStretch.h PeakFinder.h

lib_LTLIBRARIES=libSoundTouch.la
#
libSoundTouch_la_SOURCES=AAFilter.cpp FIRFilter.cpp FIFOSampleBuffer.cpp    \
    RateTransposer.cpp SoundTouch.cpp TDStretch.cpp cpu_detect_x86.cpp      \
    BPMDetect.cpp PeakFinder.cpp InterpolateLinear.cpp InterpolateCubic.cpp \
    InterpolateShannon.cpp

# Compiler flags
AM_CXXFLAGS+=-O3

# Compile the files that need MMX and SSE individually.
libSoundTouch_la_LIBADD=libSoundTouchMMX.la libSoundTouchSSE.la
noinst_LTLIBRARIES=libSoundTouchMMX.la libSoundTouchSSE.la
libSoundTouchMMX_la_SOURCES=mmx_optimized.cpp
libSoundTouchSSE_la_SOURCES=sse_optimized.cpp

# We enable optimizations by default.
# If MMX is supported compile with -mmmx.
# Do not assume -msse is also supported.
if HAVE_MMX
libSoundTouchMMX_la_CXXFLAGS = -mmmx $(AM_CXXFLAGS)
else
libSoundTouchMMX_la_CXXFLAGS = $(AM_CXXFLAGS)
endif

# We enable optimizations by default.
# If SSE is supported compile with -msse.
if HAVE_SSE
libSoundTouchSSE_la_CXXFLAGS = -msse $(AM_CXXFLAGS)
else
libSoundTouchSSE_la_CXXFLAGS = $(AM_CXXFLAGS)
endif

# Let the user disable optimizations if he wishes to.
if !X86_OPTIMIZATIONS
libSoundTouchMMX_la_CXXFLAGS = $(AM_CXXFLAGS)
libSoundTouchSSE_la_CXXFLAGS = $(AM_CXXFLAGS)
endif

# Modify the default 0.0.0 to LIB_SONAME.0.0
libSoundTouch_la_LDFLAGS=-version-info @LIB_SONAME@

# other linking flags to add
# noinst_LTLIBRARIES = libSoundTouchOpt.la
# libSoundTouch_la_LIBADD = libSoundTouchOpt.la
# libSoundTouchOpt_la_SOURCES = mmx_optimized.cpp sse_optimized.cpp 
# libSoundTouchOpt_la_CXXFLAGS = -O3 -msse -fcheck-new -I../../include
