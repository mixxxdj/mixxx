/***************************************************************************
                          light.h  -  description
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

#ifndef LIGHT_H
#define LIGHT_H

/**
 * A Light.
 * This class encapsulates an openGL lightsource into an easy
 * to use interface.
 */
class Light
{
public:
  Light();
  virtual ~Light();
  void enable();
  void disable();

  float ambient[4];         ///< The ambient color rgba.
  float diffuse[4];         ///< The diffuse color rgba.
  float specular[4];        ///< The specular color rgba.
  float position[4];        ///< The position of the light source.
  
private:
  static int nextLight;   ///< The next light number.
  int i;                  ///< The light number of this light.

protected:
  int getLightIndex();

};
#endif


