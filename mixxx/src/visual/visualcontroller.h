/***************************************************************************
                          visualcontroller.h  -  description
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

#ifndef VISUALCONTROLLER_H
#define VISUALCONTROLLER_H
#if defined(WIN32)
#include <windows.h>
#endif
#include <list>
#include <GL/gl.h>

/**
 * Forward Declaration.
 */
class VisualObject;

/**
 * Common Data Types.
 */
typedef std::list<VisualObject *> Objects;
typedef std::list<VisualObject *>::iterator ObjectsIterator;

/**
 *
 */
class VisualController
{
public: 
  VisualController();

private:
  void setupBackfaceCulling();
  void setupZBuffer();
  void setupBlending();

public:
  void init();
  void drawScene(GLenum mode);
  void display();
  void resize(GLsizei _width,GLsizei _height);
  void add(VisualObject * obj);
  void remove(VisualObject * obj);

  GLdouble fov,aspect,znear,zfar;
  GLdouble eyex,eyey,eyez;
  GLdouble centerx,centery,centerz;
  GLdouble upx,upy,upz;

  GLint x,y;
  GLsizei width,height;

private:
  Objects objects;

};
#endif

