/***************************************************************************
                          visualdata.h  -  description
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

#ifndef VISUALDATA_H
#define VISUALDATA_H

#include "visualobject.h"
#include "signalvertexbuffer.h"

class FastVertexArray;

/**
 * Visual Data.
 */
class VisualData : public VisualObject
{
public:
    VisualData(GLfloat *,FastVertexArray *);
    ~VisualData();
    void draw(GLenum mode);
    virtual void draw() = 0;

    void setOrigo(float ox, float oy,float oz);
    void setLength(float length);
    void setHeight(float height);
    void setRotation(float angle, float rx,float ry,float rz);
    void setVertexArray(bufInfo i);
protected:
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

