#ifndef _SEGMENTER_H
#define _SEGMENTER_H

/*
 *  Segmenter.h
 *  soundbite
 *
 *  Created by Mark Levy on 23/03/2006.
 *  Copyright 2006 Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
 *
 */

#include <vector>
#include <iostream>

using std::vector;
using std::ostream;

class Segment
{
public:
	int start;		// in samples
	int end;
	int type;
};

class Segmentation
{
public:
	int nsegtypes;		// number of segment types, so possible types are {0,1,...,nsegtypes-1}
	int samplerate;
	vector<Segment> segments;	
};

ostream& operator<<(ostream& os, const Segmentation& s);

class Segmenter
{
public:
	Segmenter() {}
	virtual ~Segmenter() {}
	virtual void initialise(int samplerate) = 0;	// must be called before any other methods
	virtual int getWindowsize() = 0;				// required window size for calls to extractFeatures()
	virtual int getHopsize() = 0;					// required hop size for calls to extractFeatures()
	virtual void extractFeatures(const double* samples, int nsamples) = 0;
	virtual void segment() = 0;						// call once all the features have been extracted
	virtual void segment(int m) = 0;				// specify desired number of segment-types
	virtual void clear() { features.clear(); }
	const Segmentation& getSegmentation() const { return segmentation; } 
protected:
	vector<vector<double> > features;
	Segmentation segmentation;
	int samplerate;
};

#endif
