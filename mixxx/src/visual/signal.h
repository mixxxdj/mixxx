#ifndef __VISUAL_SIGNAL_INCLUDED__
#define __VISUAL_SIGNAL_INCLUDED__

#include "visual.h"
#include "buffer.h"

class CFastVertexArray;

/**
 * A Visual Signal.
 */
class CVisualSignal : public CVisualObject
{
public:

  CVisualSignal(GLfloat *,CFastVertexArray *);
  ~CVisualSignal();

public:

  void draw(GLenum mode);
  void draw();

  void setOrigo(float ox, float oy,float oz);
  void setLength(float length);
  void setHeight(float height);
  void setRotation(float angle, float rx,float ry,float rz);
  void setVertexArray(bufInfo i);

private:

  float ox,oy,oz;   ///< Origio of visual signal (from where signal propagates from).
  float angle;      ///< Rotation angle in radians.
  float rx,ry,rz;   ///< Rotation Axe.

  bufInfo bufferInfo; ///< Pointers to vertex array

  float length;     ///< Signal Length.
  float height;     ///< Signal Heigth.

  CFastVertexArray *vertex;
  GLfloat *buffer;

};/*End class CVisualSignal*/
#endif //__VISUAL_SIGNAL_INCLUDED__

