/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
   Vamp Tempogram Plugin
   Carl Bussey, Centre for Digital Music, Queen Mary University of London
   Copyright 2014 Queen Mary University of London.
    
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.  See the file
   COPYING included with this distribution for more information.
*/

#ifndef __Tempogram__WindowFunction__
#define __Tempogram__WindowFunction__

#include <iostream>
#include <cmath>
#include <vector>

class WindowFunction{
public:
    static void hanning(float *signal, const unsigned int &N, const bool &normalise = false);
};

#endif /* defined(__Tempogram__WindowFunction__) */
