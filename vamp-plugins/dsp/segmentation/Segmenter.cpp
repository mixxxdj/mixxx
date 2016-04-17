/*
 *  Segmenter.cpp
 *
 *  Created by Mark Levy on 04/04/2006.
 *  Copyright 2006 Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
 *
 */

#include <iomanip>

#include "Segmenter.h"

ostream& operator<<(ostream& os, const Segmentation& s)
{
	os << "structure_name : begin_time end_time\n";
	
	for (int i = 0; i < s.segments.size(); i++)
	{
		Segment seg = s.segments[i];
		os << std::fixed << seg.type << ':' << '\t' << std::setprecision(6) << seg.start / static_cast<double>(s.samplerate) 
			<< '\t' << std::setprecision(6) << seg.end / static_cast<double>(s.samplerate) << "\n";
	}
	
	return os;
}
