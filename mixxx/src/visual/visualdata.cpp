/***************************************************************************
                          visualdata.cpp  -  description
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

#include "visualdata.h"
#include "fastvertexarray.h"


/**
 * Default Constructor
 */
VisualData::VisualData(GLfloat *_buffer, FastVertexArray *_vertex)
{
    buffer = _buffer;
    vertex = _vertex;
    
    ox = oy = oz = 0;
    angle = 0;
    rx = 1;ry=0;rz=0;

    bufferInfo.len1 = 0;
    bufferInfo.len2 = 0;
    length = 0;
};

/**
 * Deconstructor.
 */
VisualData::~VisualData()
{
};

/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void VisualData::draw(GLenum mode)
{
    VisualObject::draw(mode);
};

/**
 * Set Signal Origo.
 */
void VisualData::setOrigo(float ox, float oy,float oz)
{
    this->ox = ox;
    this->oy = oy;
    this->oz = oz;
};

/**

 *
 */
void VisualData::setLength(float length)
{
    this->length = length;
};

/**
 *
 */
void VisualData::setHeight(float height)
{
    this->height = height;
};

/**
 *
 */
void VisualData::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
};

/**
 *
 */
void VisualData::setVertexArray(bufInfo i)
{
    bufferInfo = i;
    // std::cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2 << "\n";
};

