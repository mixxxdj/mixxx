#ifndef __VISUAL_CONTROLLER_INCLUDED__
#define __VISUAL_CONTROLLER_INCLUDED__
#if defined(WIN32)
#include <windows.h>
#endif
#include <list>
#include <GL/gl.h>

/**
 * Forward Declaration.
 */
class CVisualObject;

/**
 * Common Data Types.
 */
typedef std::list<CVisualObject *> Objects;
typedef std::list<CVisualObject *>::iterator ObjectsIterator;

/**
 *
 */
class CVisualController
{
public: 

  CVisualController();

private:

  void setupBackfaceCulling();
  void setupZBuffer();
  void setupBlending();

public:

  void init();
  void drawScene(GLenum mode);
  void display();
  void resize(GLsizei _width,GLsizei _height);

public:
  
  void add(CVisualObject * obj);
  void remove(CVisualObject * obj);

public:

  GLdouble fov,aspect,znear,zfar;
  GLdouble eyex,eyey,eyez;
  GLdouble centerx,centery,centerz;
  GLdouble upx,upy,upz;

  GLint x,y;
  GLsizei width,height;

private:

  Objects objects;

};/* End of class CVisualController */
#endif //__VISUAL_CONTROLLER_INCLUDED__

