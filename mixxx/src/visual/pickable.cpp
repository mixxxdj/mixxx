/***************************************************************************
                          pickable.cpp  -  description
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

#include "pickable.h"
#include "material.h"
#include <qgl.h>


int PickableObject::next = 1;   ///< Next free avaible index.


/**
 * Default Constructor.
 */
PickableObject::PickableObject()
{
    index = next++;
};

/**
 * Specialized Drawing Routine.
 *
 * @param mode    The rendering state.
 */
void PickableObject::draw(GLenum mode)
{
    if(material)
        material->use();
    if(mode == GL_SELECT)
        glLoadName(index);
    draw();    
};


/**
 * Retrive Unique Index.
 *
 * @return    The value of the index.
 */
int PickableObject::getIndex()
{
    return index;
};



