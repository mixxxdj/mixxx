#ifndef __GUI_SIGNAL_INCLUDED__
#define __GUI_SIGNAL_INCLUDED__
#include "visual.h"
#include "signal.h"
#include "box.h"
#include "buffer.h"


class CFastVertexArray;
class CSignalVertexBuffer;
class ControlPotmeter;

/**
 * A GUI Signal Control.
 * This class shows a GUI control corresponding
 * to a digital signal.
 *
 *
 */
class CGUISignal : public CVisualObject
{
  Q_OBJECT
public:
  CGUISignal(CSignalVertexBuffer *buffer,CFastVertexArray * vertex, const char *group);
public:

  void update(float * signal,int samples);
  void draw(GLenum mode);
  void draw();

  void setOrigo(float ox, float oy,float oz);
  float getOrigoX();
  float getOrigoY();
  float getOrigoZ();



  void setLength(float length);
  void setHeight(float height);
  void setDepth(float depth);
  float getLength();
  float getHeight();
  float getDepth();


  void setPlayPosMarkerMaterial(CMaterial * material);
  void setBoxMaterial(CMaterial * material);
  void setBoxWireMaterial(CMaterial *material);
  void setSignalMaterial(CMaterial * material);

public slots:


  void setFishEyeSignalMaterial(CMaterial * material);
  void setFishEyeMode(bool value);
  void setFishEyeLengthScale(FLOAT_TYPE scale);
  void setFishEyeSignalFraction(FLOAT_TYPE fraction);
  void setSignalScale(float scale);


  void setRotation(float angle, float rx,float ry,float rz);

private:

  void doLayout();

  float ox,oy,oz;   ///< Origio of visual signal (from where signal propagates from)

  
  float angle;      ///< Rotation angle in radians.
  float rx,ry,rz;   ///< Rotation Axe.
  

  float fishEyeLengthScale;
  float signalScale;
  bool  fishEyeMode;
  float fishEyeSignalFraction;

  float length;
  float height;
  float depth;

  CVisualBox box;
  CVisualBox playPosMarker;

  CVisualSignal preSignal,fishEyeSignal,postSignal;
  CVisualSignal signal;
  CSignalVertexBuffer *buffer;
  CMaterial *boxMaterial, *boxWireMaterial;

  CMaterial * playPosMarkerMaterial;
  
  ControlPotmeter *sliderFishEyeSignalFraction;
  ControlPotmeter *sliderFishEyeLengthScale;

};/*End class CGUISignal*/
#endif //__GUI_SIGNAL_INCLUDED__

