#include "visualbackplane.h"

/**
 * Default Constructor.
 */
VisualBackplane::VisualBackplane()
{
  ox = oy = oz = 0;

  length = 50;
  height = 40;

  mode = GL_POLYGON;

  material = 0;

  //texture.load("c:\\mixxx\\peter.png", 1, 1);
  texture.load("/home/haste/white.png", 1, 1);
	int texsize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&texsize);


};

void VisualBackplane::draw(GLenum mode)
{
  CVisualObject::draw(mode);
};

/**
 *
 */
void VisualBackplane::draw(){

glDisable(GL_BLEND);

  glPushMatrix();
  texture.use();
  glScaled(75,75,10);
  glBegin(GL_POLYGON);
    glVertex3f(1,1,-1);
	glTexCoord2f(0,1);
    
	glVertex3f(-1,1,-1);
	glTexCoord2f(0,0);
    
	glVertex3f(-1,-1,-1);
	glTexCoord2f(1,0);
    
	glVertex3f(1,-1,-1);
	glTexCoord2f(1,1);

  glEnd();
  texture.disable();
  glPopMatrix();

glEnable(GL_BLEND);
};

/**
 * Set Origo
 */
void VisualBackplane::setOrigo(float ox, float oy,float oz){
  this->ox = ox;
  this->oy = oy;
  this->oz = oz;
};


/**
 *
 */
void VisualBackplane::setLength(float length){
  this->length = length;
};

/**
 *
 */
void VisualBackplane::setHeight(float height){
  this->height = height;
};

