/***************************************************************************
                          material.h  -  description
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

#ifndef MATERIAL_H
#define MATERIAL_H

/**
 * A material.
 * This class encapsulates an openGL material into an easy to use interface.
 */
class Material
{
public:
  Material();
  virtual ~Material();
  void use();

  float ambient[4];   ///< The ambient color rgba.
  float diffuse[4];   ///< The diffuse color rgba.
  float specular[4];  ///< The specular color rgba.
  float emission[4];  ///< The emissive color rgba.
  float shininess;    ///< The shininess of this material 0..180.

};
#endif

