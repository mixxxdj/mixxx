/***************************************************************************
                          visualsignal.h  -  description
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

#ifndef VISUALSIGNAL_H
#define VISUALSIGNAL_H

#include "visualobject.h"
#include "signalvertexbuffer.h"

class FastVertexArray;

/**
 * A Visual Signal.
 */
class VisualSignal : public VisualObject
{
public:
  VisualSignal(GLfloat *,FastVertexArray *);
  ~VisualSignal();

public:
  void draw(GLenum mode);
  void draw();

  void setOrigo(float ox, float oy,float oz);
  void setLength(float length);
  void setHeight(float height);
  void setRotation(float angle, float rx,float ry,float rz);
  void setVertexArray(bufInfo i);

private:
  float ox,oy,oz;   ///< Origio of visual signal (from where signal propagates from).
  float angle;      ///< Rotation angle in radians.
  float rx,ry,rz;   ///< Rotation Axe.

  bufInfo bufferInfo; ///< Pointers to vertex array

  float length;     ///< Signal Length.
  float height;     ///< Signal Heigth.

  FastVertexArray *vertex;
  GLfloat *buffer;

};
#endif

