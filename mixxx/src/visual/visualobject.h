/***************************************************************************
                          visualobject.h  -  description
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

#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

/*
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
*/

#include <qobject.h>
#include <qgl.h>

/**
 * Forward Declaration.
 */
class Material;

/**
 * A Visual Object.
 * Every visual object must be inherited from
 * this class (or the CPickableObject). The class
 * sets up the interface that the visual controller
 * class uses.
 *
 * All you have to do is to implement the openGL drawing
 * specific stuffs in the method draw().
 *
 * Notice that the class automatically supports materials, so
 * you do not need to handle these in you drawing method, only
 * pure geometry (and textures)
 */
class VisualObject : public QObject
{
public:
  virtual void draw(GLenum mode);
  virtual void draw()=0;
  void setMaterial(Material *material);

protected:
  Material *material; ///< The material of the object.

};
#endif

