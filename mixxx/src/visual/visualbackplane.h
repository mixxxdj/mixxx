#ifndef VISUALBACKPLANE_H
#define VISUALBACKPLANE_H

#include "visual.h"
#include "texture.h"
/**
 * A Visual Backplane with texture
 */
class VisualBackplane : public CVisualObject
{
public:

  VisualBackplane();

public:

  void draw(GLenum mode);
  void draw();
  void setDrawMode(GLenum);
  void setOrigo(float ox, float oy,float oz);
  void setLength(float length);
  void setHeight(float height);

private:

  float ox,oy,oz;   ///< Origo of the plane

  float length;     ///< The edge size in the x-axe direction.
  float height;     ///< The edge size in the y-axe direction.
  GLenum mode;
  CTexture texture;

};
#endif // VISUALBACKPLANE_H

