/***************************************************************************
                          material.cpp  -  description
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

#include "material.h"
#include <qgl.h>

/**
 * Default constructor.
 */
Material::Material(){
  ambient[0]  = 0.0f;
  ambient[1]  = 0.0f;
  ambient[2]  = 0.0f;
  ambient[3]  = 0.0f;

  diffuse[0]  = 0.0f;
  diffuse[1]  = 0.0f;
  diffuse[2]  = 0.0f;
  diffuse[3]  = 1.0f;
  
  specular[0] = 0.0f;
  specular[1] = 0.0f;
  specular[2] = 0.0f;
  specular[3] = 1.0f;

  emission[0] = 0.0f;
  emission[1] = 0.0f;

  emission[2] = 0.0f;
  emission[3] = 1.0f;

  shininess  = 0;
};

/**
 * Deconstructor.
 */
Material::~Material()
{
};

/**
 * Uses the material.
 * This method should be invoked when you want to use the material. The method makes sure to tell openGL all the parameters that the data members of this class specifies.
 */
void Material::use(){
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,ambient);
  glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,diffuse);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specular);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,emission);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&shininess);
};
