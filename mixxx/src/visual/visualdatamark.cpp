/***************************************************************************
                          visualdatamark.cpp  -  description
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

#include "visualdatamark.h"
#include "fastvertexarray.h"


/**
 * Default Constructor
 */
VisualDataMark::VisualDataMark(GLfloat *buffer, FastVertexArray *vertex) : VisualData(buffer, vertex)
{
};

/**
 * Deconstructor.
 */
VisualDataMark::~VisualDataMark()
{
};

/**
 * Draw Visual Data marks.
 * Note that the update method should have been invoked
 * before to this method. If not the data will not
 * change!!!
 */
void VisualDataMark::draw()
{
//    glDisable(GL_BLEND);
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
//    glEnable(GL_BLEND);
};

