/////////////////////////////////////////////////////////////////////////////
// Name:        InverseRIAAFilter.cpp
// Purpose:		correction of a RIAA filtered input signal.
// Author:      Stefan Langhammer
// Modified by:
// Created:     11.08.2006
// Modified:    11.08.2006
// Copyright:   (c) Stefan Langhammer
// Licence:     LGPL
/////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
//	InverseRIAAFilter.cpp - 
// -----------------------------------------------------------------------------
#include "InverseRIAAFilter.h"

	// inverted filter for phono playback (44/48kHz):
//
//  b = [ 0.2275882473429072 -0.1680644758323426 -0.0408560856583673 ]
//  a = [ 1.0000000000000000 -1.7160778983199925  0.7179700042784745 ]

float b0 =  0.2275882473429072;
float b1 = -0.1680644758323426;
float b2 = -0.0408560856583673;
float a0 =  1.0000000000000000;
float a1 =  1.7160778983199925;
float a2 = -0.7179700042784745;

float stereo_float::l = 0.0f;
float stereo_float::r = 0.0f;

// -----------------------------------------------------------------------------
// InverseRIAAFilter - ctor
// -----------------------------------------------------------------------------
InverseRIAAFilter::InverseRIAAFilter()
{
	// init x and y arrays:
	for (int i=0; i<3; i++) { x[i].l = 0.0f; x[i].r = 0.0f; y[i].l = 0.0f; y[i].r = 0.0f; }
}

// -----------------------------------------------------------------------------
// InverseRIAAFilter - dtor
// -----------------------------------------------------------------------------
InverseRIAAFilter::~InverseRIAAFilter()
{
}

// -----------------------------------------------------------------------------
//	inv_riaa_filter -	a reverse RIAA filter function which works for stereo 
//						channels at 44.1/48khz
// -----------------------------------------------------------------------------

void InverseRIAAFilter::inv_riaa_filter(short* buffer, int size)
{
	// pointers to left/right channel values
	short *bufLeft	= buffer;
	short *bufRight = bufLeft;
		   bufRight++;

    //  We just have to set x[0] and read y[0]
	for (int i = 0; i < size; i+=2)
	{
		// assign new input samples
		x[0].l = *bufLeft;
		x[0].r = *bufRight;

		// apply filter to left channel:
		y[0].l = b2*x[2].l
				+ b1*x[1].l
				+ b0*x[0].l
				+ a2*y[2].l
				+ a1*y[1].l;

		// apply filter to right channel:
		y[0].r = b2*x[2].r
				+ b1*x[1].r
				+ b0*x[0].r
				+ a2*y[2].r
				+ a1*y[1].r;

		// shift x values "away":
		x[2] = x[1];
		x[1] = x[0];

		// shift y values "away":
		y[2] = y[1];
		y[1] = y[0];

		// assign new input samples
		*bufLeft   = y[0].l;
		*bufRight  = y[0].r;

		// increase pointer position
		bufLeft++;
		bufRight = bufLeft;
		bufRight++;
	}
}
