/***************************************************************************
                          visualdatamark.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny 
                                       Erleben
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

#ifndef VISUALDATAMARK_H
#define VISUALDATAMARK_H

#include "visualdata.h"
#include "signalvertexbuffer.h"

class FastVertexArray;

/**
 * A Visual Signal.
 */
class VisualDataMark : public VisualData
{
public:
  VisualDataMark(GLfloat *,FastVertexArray *);
  ~VisualDataMark();
  void draw();
};

#endif

