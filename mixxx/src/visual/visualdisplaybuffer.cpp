/***************************************************************************
                          visualdisplaybuffer.cpp  -  description
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

#include "visualdisplaybuffer.h"

VisualDisplayBuffer::VisualDisplayBuffer(VisualBuffer *pVisualBuffer)
{
    m_pVisualBuffer = pVisualBuffer;
    
    ox = oy = oz = 0;
    angle = 0;
    rx = 1;ry=0;rz=0;

    bufferInfo.len1 = 0;
    bufferInfo.len2 = 0;
    length = 0;
}

VisualDisplayBuffer::~VisualDisplayBuffer()
{
}

void VisualDisplayBuffer::draw(GLenum mode)
{
    VisualObject::draw(mode);
}

void VisualDisplayBuffer::draw()
{
//    glDisable(GL_BLEND);
    //--- matrix mode must be  GL_MODEL
    glPushMatrix();
    glTranslatef(ox,oy,oz);
    float angle=0;
    if(angle!=0)
        glRotatef(angle,rx,ry,rz);
    float yscale = height/4.;

    //std::cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2  << "\n";
    //qDebug("LENGTH: %i",bufferInfo.len1+bufferInfo.len2);

    float xscale = length/(bufferInfo.len1+bufferInfo.len2-1);
    glScalef(xscale,yscale,1);

    //--- Now draw the mother fucker:-)
    if (bufferInfo.len1>0)
    {
        glTranslatef(-bufferInfo.p1[0],0,0);
        m_pVisualBuffer->draw(bufferInfo.p1,bufferInfo.len1);
    }

    if (bufferInfo.len2>0)
    {
        if (bufferInfo.len1==0)
           glTranslatef(-bufferInfo.p2[0],0,0);
        else
            glTranslatef(bufferInfo.p1[0]+bufferInfo.len1,0,0);
        m_pVisualBuffer->draw(bufferInfo.p2, bufferInfo.len2);
    }

    //--- Clean up after us
    glPopMatrix();
//    glEnable(GL_BLEND);

}

void VisualDisplayBuffer::setOrigo(float _ox, float _oy,float _oz)
{
    ox = _ox;
    oy = _oy;
    oz = _oz;
}

float VisualDisplayBuffer::getOrigoX()
{
    return ox;
}

float VisualDisplayBuffer::getOrigoY()
{
    return oy;
}

float VisualDisplayBuffer::getOrigoZ()
{
    return oz;
}

void VisualDisplayBuffer::setLength(float _length)
{
    length = _length;
}

float VisualDisplayBuffer::getLength()
{
    return length;
}

void VisualDisplayBuffer::setHeight(float _height)
{
    height = _height;
}

float VisualDisplayBuffer::getHeight()
{
    return height;
}

void VisualDisplayBuffer::setDepth(float _depth)
{
    depth = _depth;
}

float VisualDisplayBuffer::getDepth()
{
    return depth;
}

void VisualDisplayBuffer::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
}

void VisualDisplayBuffer::setBuffer(bufInfo i)
{
    bufferInfo = i;
    // std::cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2 << "\n";
}


