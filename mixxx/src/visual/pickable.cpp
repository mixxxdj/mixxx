#include "pickable.h"
#include "material.h"
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>


int CPickableObject::next = 1;   ///< Next free avaible index.


/**
 * Default Constructor.
 */
CPickableObject::CPickableObject(){
  index = next++;
};

/**
 * Specialized Drawing Routine.
 *
 * @param mode    The rendering state.
 */
void CPickableObject::draw(GLenum mode){
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
int CPickableObject::getIndex(){
  return index;
};



