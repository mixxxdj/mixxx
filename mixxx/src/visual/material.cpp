#include "material.h"
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

/**
 * Default constructor.
 */
CMaterial::CMaterial(){
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
CMaterial::~CMaterial(){};

/**
 * Uses the material.
 * This method should be invoked when you want to use the material. The method makes sure to tell openGL all the parameters that the data members of this class specifies.
 */
void CMaterial::use(){
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,ambient);
  glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,diffuse);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specular);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,emission);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&shininess);
};
