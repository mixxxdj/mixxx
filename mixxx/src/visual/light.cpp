#include "light.h"
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

int CLight::nextLight = 1;

/**
 * Default constructor.
 */
CLight::CLight(){
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
CLight::~CLight(){
  glDisable(getLightIndex());
};

	/**
 * Disables a light source.
 * This mehod disables this light. Note lighting is not disabled
 * only this single lightsource is disabled.
 */
void CLight::disable(){
  glDisable(getLightIndex());
};

/**
 * Enables a light source.
 * This method enables both lighting  and this light source.
 */
void CLight::enable(){

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();


  glShadeModel(GL_SMOOTH);
  
  if(!glIsEnabled(GL_LIGHTING))
    glEnable(GL_LIGHTING);
  if(!glIsEnabled(getLightIndex()))
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
int CLight::getLightIndex(){
  return GL_LIGHT0 + i;
};
