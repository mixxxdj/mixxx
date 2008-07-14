/***************************************************************************
                          picking.h  -  description
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

#ifndef PICKING_H
#define PICKING_H

#include <qgl.h>

/**
 * Forward Declaration.
 */
class VisualController;

#define BUFSIZE 512

/**
 * Picking Support Class.
 * This class can be used to pick objects.
 *
 * You use it by invoking the method pick(...), which
 * return a unique index of a pickable object.
 */
class Picking
{
public:
  Picking();
  void init(VisualController * controller);

private:
  GLuint inFront(GLint hits, GLuint buffer[]);

public:
  int pick(int x,int y);

private:
    GLuint selectBuf[BUFSIZE];
    GLint i,hits;
    GLint viewport[4];
    GLuint    j, num_names, picked;
    GLuint*   p;
    GLboolean save;
    GLuint    depth;

    VisualController *controller;

};
#endif


