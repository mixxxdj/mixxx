#include "visual.h"
#include "material.h"
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>


/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void CVisualObject::draw(GLenum mode){
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
void CVisualObject::setMaterial(CMaterial * material){
  this->material = material;
};
