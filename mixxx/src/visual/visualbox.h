/***************************************************************************
                          visualbox.h  -  description
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

#ifndef VISUALBOX_H
#define VISUALBOX_H
#include "pickable.h"

/**
 * A Visual Pickable Box.
 */
class VisualBox : public PickableObject
{
public:
  VisualBox(int id);

  void draw(GLenum mode);
  void draw();
  void setDrawMode(GLenum);
  void setOrigo(float ox, float oy,float oz);
  void setRotation(float angle, float rx,float ry,float rz);
  void setLength(float length);
  void setHeight(float height);
  void setDepth(float depth);

private:
  float ox,oy,oz;   ///< Origo of the box

  float angle;      ///< Rotation angle in radians.
  float rx,ry,rz;   ///< Rotation Axe.

  float length;     ///< The edge size in the x-axe direction.
  float height;     ///< The edge size in the y-axe direction.
  float depth;      ///< The edge size in the z-axe direction.

  GLenum mode;		///< Draw mode


};

#endif

