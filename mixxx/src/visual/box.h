#ifndef __VISUAL_BOX_INCLUDED__
#define __VISUAL_BOX_INCLUDED__
#include "pickable.h"

/**
 * A Visual Pickable Box.
 */
class CVisualBox : public CPickableObject
{
public:

  CVisualBox();

public:

  void draw(GLenum mode);
  void draw();
  void setDrawMode(GLenum);
  void setOrigo(float ox, float oy,float oz);
  void setRotation(float angle, float rx,float ry,float rz);
  void setLength(float length);
  void setHeight(float height);
  void setDepth(float depth);

private:

  float ox,oy,oz;   ///< Origo of the box

  float angle;      ///< Rotation angle in radians.
  float rx,ry,rz;   ///< Rotation Axe.

  float length;     ///< The edge size in the x-axe direction.
  float height;     ///< The edge size in the y-axe direction.
  float depth;      ///< The edge size in the z-axe direction.

  GLenum mode;		///< Draw mode


};/*End class CVisualBox*/
#endif //__VISUAL_BOX_INCLUDED__

