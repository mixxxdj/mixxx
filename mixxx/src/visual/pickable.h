#ifndef __PICKABLE_INCLUDED__
#define __PICKABLE_INCLUDED__
#if defined(WIN32)
#include <windows.h>
#endif
#include "visual.h"
#include <GL/gl.h>

/**
 * A Picable Visual Object.
 * This class implements the necessay openGL support
 * to pick the object.
 *
 * You can inherit you own visual object from this
 * class in order to make them pickable.
 *
 * You should use the CPicking when you want to pick
 * an object at a given location.
 */
class CPickableObject : public CVisualObject
{
public:

  CPickableObject();

public:

  virtual void draw(GLenum mode);
  virtual void draw()=0;
  int getIndex();

private:

  int index;        ///< The index of the object.
  static int next;  ///< Next free avaible index.

};/* End of class CPickableObject */
#endif //__PICKABLE_INCLUDED__

