#include "pick.h"
#ifdef _WIN32_
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "controller.h"

/**
 * Default Constructor.
 */
CPicking::CPicking()
{
}

void CPicking::init(CVisualController * controller){
  this->controller = controller;
};

/**
 * processHits prints out the contents of the selection array
 */
void CPicking::processHits(GLint hits, GLuint buffer[]){
  GLuint names, *ptr;

  //printf ("hits = %d\n", hits);
  ptr = (GLuint *) buffer;
  for(i=0;i<hits;i++) {	/*  for each hit  */
    names = *ptr;
    //printf (" number of names for hit = %d\n", names);
    ptr++;
    //printf("  z1 is %g;", (float) *ptr/0x7fffffff);
    ptr++;
    //printf(" z2 is %g\n", (float) *ptr/0x7fffffff);
    ptr++;
    //printf ("   the name is ");
    for (j=0;j<names;j++){	/*  for each name */
      //printf ("%d ", *ptr);
      ptr++;
    }
    //printf ("\n");
  }
};

/**
 * Processes hits, in order to find the one in front
 * of all the others.
 */
GLuint CPicking::inFront(GLint hits, GLuint buffer[]){

  depth = (GLuint)-1;

  p = buffer;
    
  picked = 0;
  
  for(i=0;i<hits;i++){
    save = GL_FALSE;
    num_names = *p;			/* number of names in this hit */
    p++;

    if(*p <= depth) {			/* check the 1st depth value */
      depth = *p;
      save = GL_TRUE;
    }
    p++;
    if(*p <= depth) {			/* check the 2nd depth value */
      depth = *p;
      save = GL_TRUE;
    }
    p++;

    if(save)
     picked = *p;

    p += num_names;			/* skip over the rest of the names */
  }

  return picked;
};

/**
 * Pick Object.
 * This method indentifies the object, at specified view location
 * 
 * @param x    The x-value of the pixel location.
 * @param y    The y-value of the pixel location.
 *
 * @return     An unique index, representing the picked object.
 */
int CPicking::pick(int x,int y){
//  GLenum error = glGetError();
//  const GLubyte* errmsg = gluErrorString(error);
      
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  glSelectBuffer(BUFSIZE, selectBuf);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName((unsigned)0);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  /*  create 5x5 pixel picking region near cursor location    */
  gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 5.0, 5.0, viewport);    
  gluPerspective(controller->fov,controller->aspect,controller->znear,controller->zfar);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
	  controller->eyex,controller->eyey,controller->eyez,
	  controller->centerx,controller->centery,controller->centerz,
	  controller->upx,controller->upy,controller->upz
	);
  controller->drawScene(GL_SELECT);  
  glFlush();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  hits = glRenderMode(GL_RENDER);

  int  picked = inFront(hits,selectBuf);
  return picked;
};
