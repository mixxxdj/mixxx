/***************************************************************************
                          visualsignal.cpp  -  description
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

#include "visualsignal.h"
#include "fastvertexarray.h"


/**
 * Default Constructor
 */
VisualSignal::VisualSignal(GLfloat *_buffer, FastVertexArray *_vertex)
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
VisualSignal::~VisualSignal()
{
};

/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void VisualSignal::draw(GLenum mode)
{
    VisualObject::draw(mode);
};

/**
 * Draw Visual Signal.
 * Note that the update method should have been invoked
 * before to this method. If not the signal will not
 * change!!!
 */
void VisualSignal::draw()
{
    glDisable(GL_BLEND);
    //--- matrix mode must be  GL_MODEL
    glPushMatrix();
    glTranslatef(ox,oy,oz);
    float angle=0;
    if(angle!=0)
        glRotatef(angle,rx,ry,rz);
    float yscale = height/2.0f;

    //std::cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2  << "\n";
    //qDebug("LENGTH: %i",bufferInfo.len1+bufferInfo.len2);

    float xscale = length/(bufferInfo.len1+bufferInfo.len2-1);
    glScalef(xscale,yscale,1);

    //--- Now draw the mother fucker:-)
    if (bufferInfo.len1>0)
    {
        glTranslatef(-bufferInfo.p1[0],0,0);
        vertex->draw(bufferInfo.p1,bufferInfo.len1);
    }

    if (bufferInfo.len2>0)
    {
        if (bufferInfo.len1==0)
           glTranslatef(-bufferInfo.p2[0],0,0);
        else
            glTranslatef(bufferInfo.p1[0]+bufferInfo.len1,0,0);
        vertex->draw(bufferInfo.p2, bufferInfo.len2);
    }
  
    //--- Clean up after us
    glPopMatrix();
    glEnable(GL_BLEND);
};

/**
 * Set Signal Origo.
 */
void VisualSignal::setOrigo(float ox, float oy,float oz)
{
    this->ox = ox;
    this->oy = oy;
    this->oz = oz;
};

/**

 *
 */
void VisualSignal::setLength(float length)
{
    this->length = length;
};

/**
 *
 */
void VisualSignal::setHeight(float height)
{
    this->height = height;
};

/**
 *
 */
void VisualSignal::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
};

/**
 *
 */
void VisualSignal::setVertexArray(bufInfo i)
{
    bufferInfo = i;
    // std::cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2 << "\n";
};

