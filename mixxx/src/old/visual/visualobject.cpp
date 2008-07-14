/***************************************************************************
                          visualobject.cpp  -  description
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

#include "visualobject.h"
#include "material.h"
#include <qgl.h>

VisualObject::VisualObject()
{
    material = 0;
}

/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void VisualObject::draw(GLenum mode)
{
    if(mode==GL_SELECT)
        return;
    if(material)
        material->use();
    draw();
};

/**
 * Assign Material.
 * 
 * @param material   A pointer to the material you want
 *                   to use on the object.
 */
void VisualObject::setMaterial(Material *_material)
{
    material = _material;
};
