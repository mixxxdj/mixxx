/***************************************************************************
                          pickable.h  -  description
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

#ifndef PICKABLE_H
#define PICKABLE_H
#if defined(WIN32)
#include <windows.h>
#endif
#include "visualobject.h"
#include <GL/gl.h>

/**
 * A Picable Visual Object.
 * This class implements the necessay openGL support
 * to pick the object.
 *
 * You can inherit you own visual object from this
 * class in order to make them pickable.
 *
 * You should use the CPicking when you want to pick
 * an object at a given location.
 */
class PickableObject : public VisualObject
{
public:
  PickableObject();
  virtual void draw(GLenum mode);
  virtual void draw()=0;
  int getIndex();

private:
  int index;        ///< The index of the object.
  static int next;  ///< Next free avaible index.
};
#endif

