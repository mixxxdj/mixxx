/***************************************************************************
                          visualcontroller.cpp  -  description
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

#include "visualcontroller.h"
#include "visualobject.h"
#include "light.h"

int VisualController::idCount = 0;

/**
 *
 */
 VisualController::VisualController()
{
  fov   = 60.0;
  znear = 10.0;
  zfar  = 1000.0;
  eyex  = 0.;
  eyey  = 0.0;
  eyez  = 120.0;
  centerx=centery=centerz=0;
  upx=0;
  upy=1;
  upz=0;

  x = 100;
  y = 100;
  width = 800;
  height = 600;
  if(height>0)
    aspect=(GLdouble)width/(GLdouble)height;
  else
    aspect=1.0;

/*
    mylight.ambient[0]  = 1.f;
    mylight.ambient[1]  = 1.f;
    mylight.ambient[2]  = 1.f;
    mylight.ambient[3]  = 1.f;

    mylight.diffuse[0]  = 1.f;
    mylight.diffuse[1]  = 1.f;
    mylight.diffuse[2]  = 1.f;
    mylight.diffuse[3]  = 1.f;

    mylight.specular[0] = 1.f;
    mylight.specular[1] = 1.f;
    mylight.specular[2] = 1.f;
    mylight.specular[3] = 1.f;

    mylight.position[0] = 100.0f;
    mylight.position[1] = 100.0f;
    mylight.position[2] = 180.0f;
    mylight.position[3] = 1.0f;    
*/
};


/**
 * Initializes opengl, so backface culling is enabled.
 */
void VisualController::setupBackfaceCulling()
{
    glFrontFace(GL_CCW);        //--- Counter clock-wise polygons face out
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
//  glDisable(GL_CULL_FACE);
};

/**
 * Initializes opengl, so zbuffer is enabled.
 */
void VisualController::setupZBuffer()
{
//  glDisable(GL_DEPTH_TEST);
    //glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    //glDepthMask(true);
    //glDepthFunc(GL_LESS);
    //glDepthRange(0, 1);
};

/**
 * Initializes opengl, so alphablending is enabled.
 */
void VisualController::setupBlending()
{
    glEnable(GL_BLEND);
    //--- Transparency fog effects???? primitves should be sorted from nearst to farest
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
};

/**
 * Controller Initialization Routine.
 */
void VisualController::init()
{
    //glClearColor(0.4f,0.4f,0.4f,0.0f);
    glClearColor(0.0f,0.f,0.0f,0.0f);
//    glClearColor(202.0/255.f,213.0/255.f,192.0/255.0f,0.0f);
//    glClearColor(1.0f,1.f,1.0f,0.1f);

    setupBackfaceCulling();
    setupZBuffer();
    setupBlending();

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
/*
    glHint(GL_FOG_HINT,GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_FASTEST);
    glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_FASTEST); 
*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov,aspect,znear,zfar);
    glViewport(x,y,width,height);

    mylight.enable();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyex,eyey,eyez,
              centerx,centery,centerz,
              upx,upy,upz);

};

/**
 *
 */
void VisualController::drawScene(GLenum mode)
{
    ObjectsIterator it = objects.begin();
    for(;it!=objects.end();++it)
        (*it)->draw(mode);
};

/**
 * Display Function.
 */
void VisualController::display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eyex,eyey,eyez,
              centerx,centery,centerz,
              upx,upy,upz);

    glDrawBuffer(GL_BACK);
    drawScene(GL_RENDER);
    glFlush();
};

/**
 * Resize Function.
 */
void VisualController::resize(GLsizei _width,GLsizei _height)
{
    width = _width;
    height = _height;
    
    if(height>0)
        aspect=(GLdouble)width/(GLdouble)height;
    else
        aspect=1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    glOrtho(-10, 10, -10, 10, znear, zfar);
    gluPerspective(fov,aspect,znear,zfar);
    glViewport(0,0,width,height);
};
  
int VisualController::add(VisualObject * obj)
{
    objects.push_back(obj);
    int id=idCount;
    qDebug("id %i",id);
    idCount++;

    return id;
};


void VisualController::remove(VisualObject * obj)
{
    objects.remove(obj);
};
