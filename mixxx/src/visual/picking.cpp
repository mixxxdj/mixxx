/***************************************************************************
                          picking.cpp  -  description
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

#include "picking.h"
#include "visualcontroller.h"

/**
 * Default Constructor.
 */
Picking::Picking()
{
    controller = 0;
}

void Picking::init(VisualController * controller)
{
    this->controller = controller;
};

/**
 * processHits prints out the contents of the selection array
 */
void Picking::processHits(GLint hits, GLuint buffer[])
{
  GLuint names, *ptr;

  //printf ("hits = %d\n", hits);
  ptr = (GLuint *) buffer;
  for(i=0;i<hits;i++) {	/*  for each hit  */
    names = *ptr;
    //printf (" number of names for hit = %d\n", names);
    ptr++;
    //printf("  z1 is %g;", (float) *ptr/0x7fffffff);
    ptr++;
    //printf(" z2 is %g\n", (float) *ptr/0x7fffffff);
    ptr++;
    //printf ("   the name is ");
    for (j=0;j<names;j++){	/*  for each name */
      //printf ("%d ", *ptr);
      ptr++;
    }
    //printf ("\n");
  }
};

/**
 * Processes hits, in order to find the one in front
 * of all the others.
 */
GLuint Picking::inFront(GLint hits, GLuint buffer[])
{

  depth = (GLuint)-1;

  p = buffer;
    
  picked = 0;
  
  for(i=0;i<hits;i++){
    save = GL_FALSE;
    num_names = *p;			/* number of names in this hit */
    p++;

    if(*p <= depth) {			/* check the 1st depth value */
      depth = *p;
      save = GL_TRUE;
    }
    p++;
    if(*p <= depth) {			/* check the 2nd depth value */
      depth = *p;
      save = GL_TRUE;
    }
    p++;

    if(save)
     picked = *p;

    p += num_names;			/* skip over the rest of the names */
  }

  return picked;
};

/**
 * Pick Object.
 * This method indentifies the object, at specified view location
 * 
 * @param x    The x-value of the pixel location.
 * @param y    The y-value of the pixel location.
 *
 * @return     An unique index, representing the picked object.
 */
int Picking::pick(int x,int y)
{
//  GLenum error = glGetError();
//  const GLubyte* errmsg = gluErrorString(error);

    int picked = 0;  
    if (controller != 0)
    {
        glGetIntegerv(GL_VIEWPORT, viewport);
  
        glSelectBuffer(BUFSIZE, selectBuf);
        glRenderMode(GL_SELECT);
        glInitNames();
        glPushName((unsigned)0);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        /*  create 5x5 pixel picking region near cursor location    */
        gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 5.0, 5.0, viewport);    
        gluPerspective(controller->fov,controller->aspect,controller->znear,controller->zfar);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(controller->eyex,controller->eyey,controller->eyez,
                  controller->centerx,controller->centery,controller->centerz,
                  controller->upx,controller->upy,controller->upz);
        controller->drawScene(GL_SELECT);  
        glFlush();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        hits = glRenderMode(GL_RENDER);

        picked = inFront(hits,selectBuf);
    }
    return picked;
};

