/***************************************************************************
                          visualbox.cpp  -  description
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

#include "visualbox.h"
#include <qgl.h>
#include <math.h>

/**
 * Default Constructor.
 */
VisualBox::VisualBox()
{
    ox = oy = oz = 0;

    angle = 0;
    rx = 1;ry=0;rz=0;
  
    length = 50;
    height = 40;
    depth  = 30;

    mode = GL_POLYGON;
};

/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void VisualBox::draw(GLenum mode)
{
    PickableObject::draw(mode);
};


/**
 *
 */
void VisualBox::draw(){

  //--- matrix mode must be  GL_MODEL
  glPushMatrix();
  glTranslatef(ox,oy,oz);
  float angle=0;
  if(angle!=0){
    glRotatef(angle,rx,ry,rz);
  }
  float xscale = length/2.0f;

  glTranslatef(xscale,0,0);

  float yscale = height/2.0f;
  float zscale = depth/2.0f;
  glScalef(xscale,yscale,zscale);
    
  glBegin(mode);
    glVertex3f(1,1,1);
    glVertex3f(1,1,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,1,1);
  glEnd();
  glBegin(mode);
    glVertex3f(-1,-1,1);
    glVertex3f(-1,1,1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,-1,-1);
  glEnd();

  glBegin(mode);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(1,1,-1);
    glVertex3f(1,-1,-1);
  glEnd();
  glBegin(mode);
    glVertex3f(-1,-1,1);
    glVertex3f(1,-1,1);
    glVertex3f(1,1,1);
    glVertex3f(-1,1,1);
  glEnd();  

  glBegin(mode);
    glVertex3f(1,-1,-1);
    glVertex3f(1,1,-1);
    glVertex3f(1,1,1);
    glVertex3f(1,-1,1);
  glEnd();

  glBegin(mode);
    glVertex3f(-1,-1,1);
    glVertex3f(-1,-1,-1);
    glVertex3f(1,-1,-1);
    glVertex3f(1,-1,1);
  glEnd();
/*
  // Playpos marker
  glBegin(mode);
    glVertex3f(0,-1,1);
    glVertex3f(0,1,1);
    glVertex3f(0,1,-1);
    glVertex3f(0,-1,-1);
  glEnd();
*/
  glPopMatrix();
};

void VisualBox::setDrawMode(GLenum _mode)
{
    mode = _mode;
}

/**
 * Set Origo
 */
void VisualBox::setOrigo(float ox, float oy,float oz)
{
    this->ox = ox;
    this->oy = oy;
    this->oz = oz;
};


/**
 *
 */
void VisualBox::setLength(float length)
{
    this->length = length;
};

/**
 *
 */
void VisualBox::setHeight(float height)
{
    this->height = height;
};

/**
 *
 */
void VisualBox::setDepth(float depth)
{
    this->depth = depth;
};

/**
 * Set Rotation.
 */
void VisualBox::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
};
