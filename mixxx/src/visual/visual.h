#ifndef __VISUAL_OBJECT_INCLUDED__
#define __VISUAL_OBJECT_INCLUDED__

/*
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
*/

#include <qobject.h>
#include <qgl.h>

/**
 * Forward Declaration.
 */
class CMaterial;

/**
 * A Visual Object.
 * Every visual object must be inherited from
 * this class (or the CPickableObject). The class
 * sets up the interface that the visual controller
 * class uses.
 *
 * All you have to do is to implement the openGL drawing
 * specific stuffs in the method draw().
 *
 * Notice that the class automatically supports materials, so
 * you do not need to handle these in you drawing method, only
 * pure geometry (and textures)
 */
class CVisualObject : public QObject
{
public:
  virtual void draw(GLenum mode);
  virtual void draw()=0;
  void setMaterial(CMaterial * material);

protected:

  CMaterial * material; ///< The material of the object.

};/* End of class CVisualObject */
#endif //__VISUAL_OBJECT_INCLUDED__

