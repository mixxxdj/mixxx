#include "box.h"
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <math.h>

/**
 * Default Constructor.
 */
CVisualBox::CVisualBox()
{
  ox = oy = oz = 0;

  angle = 0;
  rx = 1;ry=0;rz=0;
  
  length = 50;
  height = 40;
  depth  = 30;

  mode = GL_POLYGON;
};

/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void CVisualBox::draw(GLenum mode)
{
  CPickableObject::draw(mode);
};


/**
 *
 */
void CVisualBox::draw(){

  //--- matrix mode must be  GL_MODEL
  glPushMatrix();
  glTranslatef(ox,oy,oz);
  float angle=0;
  if(angle!=0){
    glRotatef(angle,rx,ry,rz);
  }
  float xscale = length/2.0f;

  glTranslatef(xscale,0,0);

  float yscale = height/2.0f;
  float zscale = depth/2.0f;
  glScalef(xscale,yscale,zscale);
    
  glBegin(mode);
    glVertex3f(1,1,1);
    glVertex3f(1,1,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,1,1);
  glEnd();
  glBegin(mode);
    glVertex3f(-1,-1,1);
    glVertex3f(-1,1,1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,-1,-1);
  glEnd();

  glBegin(mode);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(1,1,-1);
    glVertex3f(1,-1,-1);
  glEnd();
  glBegin(mode);
    glVertex3f(-1,-1,1);
    glVertex3f(1,-1,1);
    glVertex3f(1,1,1);
    glVertex3f(-1,1,1);
  glEnd();  

  glBegin(mode);
    glVertex3f(1,-1,-1);
    glVertex3f(1,1,-1);
    glVertex3f(1,1,1);
    glVertex3f(1,-1,1);
  glEnd();

  glBegin(mode);
    glVertex3f(-1,-1,1);
    glVertex3f(-1,-1,-1);
    glVertex3f(1,-1,-1);
    glVertex3f(1,-1,1);
  glEnd();
/*
  // Playpos marker
  glBegin(mode);
    glVertex3f(0,-1,1);
    glVertex3f(0,1,1);
    glVertex3f(0,1,-1);
    glVertex3f(0,-1,-1);
  glEnd();
*/
  glPopMatrix();
};

void CVisualBox::setDrawMode(GLenum _mode)
{
	mode = _mode;
}

/**
 * Set Origo
 */
void CVisualBox::setOrigo(float ox, float oy,float oz){
  this->ox = ox;
  this->oy = oy;
  this->oz = oz;
};


/**
 *
 */
void CVisualBox::setLength(float length){
  this->length = length;
};

/**
 *
 */
void CVisualBox::setHeight(float height){
  this->height = height;
};

/**
 *
 */
void CVisualBox::setDepth(float depth){
  this->depth = depth;
};

/**
 * Set Rotation.
 */
void CVisualBox::setRotation(float angle, float rx,float ry,float rz){
  this->angle = angle;
  this->rx = rx;
  this->ry = ry;
  this->rz = rz;
};
