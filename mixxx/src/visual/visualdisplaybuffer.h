/***************************************************************************
                          visualdisplaybuffer.h  -  description
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

#ifndef VISUALDISPLAYBUFFER_H
#define VISUALDISPLAYBUFFER_H

#include <qgl.h>
#include "visualobject.h"
#include "visualbuffer.h"

/**
 * Visual Data.
 */
class VisualDisplayBuffer : public VisualObject
{
public:
    /**
     * Initialize Vertex Array.
     *
     * Developers Note: It seems that there is
     * a upper limit on the size of data that can
     * be within a fence. Measurements show that
     * around 200000 bytes the drawing looks akward???
     *
     * @param vertices            The number of vertices
     * @param bufferCount         The number of wanted buffers
     */
    VisualDisplayBuffer(VisualBuffer *pVisualBuffer);
    ~VisualDisplayBuffer();
    void draw(GLenum mode);
    void draw();
    /** Set Signal Origo. */
    void setOrigo(float ox, float oy,float oz);
    float getOrigoX();
    float getOrigoY();
    float getOrigoZ();
    void setLength(float length);
    float getLength();
    void setHeight(float height);
    float getHeight();
    void setDepth(float depth);
    float getDepth();
    void setRotation(float angle, float rx,float ry,float rz);
    void setBuffer(bufInfo i);

protected:
    /** Pointer to associated VisualBuffer */
    VisualBuffer *m_pVisualBuffer;

    float ox,oy,oz;   ///< Origio of visual signal (from where signal propagates from).
    float angle;      ///< Rotation angle in radians.
    float rx,ry,rz;   ///< Rotation Axe.

    bufInfo bufferInfo; ///< Pointers to vertex array

    float length;     ///< Signal Length.
    float height;     ///< Signal Heigth.
    float depth;


private:
};

#endif

