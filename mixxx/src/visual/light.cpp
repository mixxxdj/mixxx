/***************************************************************************
                          light.cpp  -  description
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

#include "light.h"
#include <qgl.h>

int Light::nextLight = 1;

/**
 * Default constructor.
 */
Light::Light()
{
    i = nextLight;
    nextLight++;

    ambient[0]  = 0.5f;
    ambient[1]  = 0.5f;
    ambient[2]  = 0.5f;
    ambient[3]  = 1.0f;
    diffuse[0]  = 0.35f;
    diffuse[1]  = 0.35f;
    diffuse[2]  = 0.35f;
    diffuse[3]  = 1.0f;
    specular[0] = 1.0f;
    specular[1] = 1.0f;
    specular[2] = 1.0f;
    specular[3] = 1.0f;
    position[0] = 0.0f;
    position[1] = 0.0f;
    position[2] = 250.0f;
};

/**
 * Deconstructor.
 */
Light::~Light()
{
    glDisable(getLightIndex());
};

	/**
 * Disables a light source.
 * This mehod disables this light. Note lighting is not disabled
 * only this single lightsource is disabled.
 */
void Light::disable()
{
    glDisable(getLightIndex());
};

/**
 * Enables a light source.
 * This method enables both lighting  and this light source.
 */
void Light::enable()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
  
    if (!glIsEnabled(GL_LIGHTING))
        glEnable(GL_LIGHTING);
    if (!glIsEnabled(getLightIndex()))
        glEnable(getLightIndex());

    glLightfv(getLightIndex(),GL_AMBIENT,ambient);
    glLightfv(getLightIndex(),GL_DIFFUSE,diffuse);
    glLightfv(getLightIndex(),GL_SPECULAR,specular);
    glLightfv(getLightIndex(),GL_POSITION,position);

    glPopMatrix();
};

/**
 * Queries the openGL light index.
 * This method returns the proper value of the light index, as opengl uses it.
 *
 * @return   The openGL light index.
 */
int Light::getLightIndex()
{
    return GL_LIGHT0 + i;
};
