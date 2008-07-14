/***************************************************************************
                          visualbackplane.h  -  description
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

#ifndef VISUALBACKPLANE_H
#define VISUALBACKPLANE_H

#include "visualobject.h"
#include "texture.h"
/**
 * A Visual Backplane with texture
 */
class VisualBackplane : public VisualObject
{
public:
  VisualBackplane();
  void draw(GLenum mode);
  void draw();
  void setDrawMode(GLenum);
  void setOrigo(float ox, float oy,float oz);
  void setLength(float length);
  void setHeight(float height);

private:
  float ox,oy,oz;   ///< Origo of the plane
  float length;     ///< The edge size in the x-axe direction.
  float height;     ///< The edge size in the y-axe direction.
  GLenum mode;
  Texture texture;
};
#endif // VISUALBACKPLANE_H

