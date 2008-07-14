/***************************************************************************
                          texture.h  -  description
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

#ifndef TEXTURE_H
#define TEXTURE_H
#include <qgl.h>

/**
 * A texture.
 * This class represents a texture map. The map
 * corresponds to a stored bmp-file.
 */
class Texture
{
public:

  Texture();
  virtual ~Texture();

  int load(char * filename,const int & wrap,const int & decal);
  int unload(void);
  void use();

  static void disable(void);
  static void enable(void);

protected:
  GLuint texture; ///< A texture object.
  int loaded;     ///< If true then m_texture is a valid texture object
  int decal;      ///< Remembers how the texture should be rendered.
  void validate();

};
#endif

