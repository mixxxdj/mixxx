## Process this file with automake to create Makefile.in
##
## $Id: Makefile.am 37 2008-12-25 16:33:39Z oparviai $
##
## Copyright (C) 2003 - David W. Durham
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
EXTRA_DIST=3dnow_win.cpp cpu_detect_x86_win.cpp SoundTouch.dsp SoundTouch.dsw SoundTouch.sln SoundTouch.vcproj

noinst_HEADERS=AAFilter.h cpu_detect.h cpu_detect_x86_gcc.cpp FIRFilter.h RateTransposer.h TDStretch.h PeakFinder.h

lib_LTLIBRARIES=libSoundTouch.la
#
libSoundTouch_la_SOURCES=AAFilter.cpp FIRFilter.cpp FIFOSampleBuffer.cpp mmx_optimized.cpp sse_optimized.cpp RateTransposer.cpp SoundTouch.cpp TDStretch.cpp cpu_detect_x86_gcc.cpp BPMDetect.cpp PeakFinder.cpp


# Note by authore: '-msse2' might not work in non-X86 compilations. If someone can
# fix this script to automatically check for CPU architecture, please submit a patch 
# to me.
AM_CXXFLAGS=-O3 -msse2 -fcheck-new -I../../include


# other linking flags to add
# noinst_LTLIBRARIES = libSoundTouchOpt.la
# libSoundTouch_la_LIBADD = libSoundTouchOpt.la
# libSoundTouchOpt_la_SOURCES = mmx_optimized.cpp sse_optimized.cpp 
# libSoundTouchOpt_la_CXXFLAGS = -O3 -msse2 -fcheck-new -I../../include
