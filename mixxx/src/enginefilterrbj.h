/***************************************************************************
                          enginefilterrbj.h  -  description
                             -------------------
    begin                : Wed Apr 3 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEFILTERRBJ_H
#define ENGINEFILTERRBJ_H

#include "engineobject.h"

/**
  *@author Tue and Ken Haste Andersen
  */


class EngineFilterRBJ : public EngineObject {
public:
    EngineFilterRBJ();
    ~EngineFilterRBJ();

//    void notify(double) {};
    CSAMPLE *process(const CSAMPLE *source, const int buf_size);
    void calc_filter_coeffs(int const type, double const frequency, double const sample_rate, double const q, double const db_gain, bool q_is_bandwidth);

private:
    // Filter coeffs
    float b0a0,b1a0,b2a0,a1a0,a2a0;

    // In/out history
    float ou1l, ou2l, in1l, in2l, ou1r, ou2r, in1r, in2r;

    CSAMPLE *buffer;
};

#endif
