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

#include "WindowFunction.h"
using std::vector;

//static function
void
WindowFunction::hanning(float * window, const unsigned int &N, const bool &normalise){
    
    float sum = 0;
    for(int i = 0; i < (int)N; i++){
        window[i] = 0.5*(1-cos((float)2*M_PI*i/N));
        sum += window[i];
    }
    if (normalise){
        for(int i = 0; i < (int)N; i++){
            window[i] /= sum;
        }
    }
}
