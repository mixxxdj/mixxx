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
    EngineFilterRBJ(bool low, CSAMPLE frequency, CSAMPLE bandwidth);
    ~EngineFilterRBJ();
    CSAMPLE *process(const CSAMPLE *source, const int buf_size);
private:
    CSAMPLE c0, c1, c2, c3, c4;

    /** Block boundary values */
    CSAMPLE x1, x2, y1, y2;

	CSAMPLE *buffer;
};

#endif
