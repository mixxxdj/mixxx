/***************************************************************************
                          guisignal.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny 
                                       Erleben
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GUISIGNAL_H
#define GUISIGNAL_H
#include "visualobject.h"
#include "visualsignal.h"
#include "visualbox.h"
#include "signalvertexbuffer.h"


class FastVertexArray;
class SignalVertexBuffer;
class ControlPotmeter;

/**
 * A GUI Signal Control.
 * This class shows a GUI control corresponding
 * to a digital signal.
 *
 *
 */
class GUISignal : public VisualObject
{
  Q_OBJECT
public:
  GUISignal(SignalVertexBuffer *buffer, FastVertexArray * vertex, const char *group);
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


  void setPlayPosMarkerMaterial(Material *material);
  void setBoxMaterial(Material *material);
  void setBoxWireMaterial(Material *material);
  void setSignalMaterial(Material *material);

public slots:
  void setFishEyeSignalMaterial(Material *material);
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

  VisualBox box;
  VisualBox playPosMarker;

  VisualSignal preSignal,fishEyeSignal,postSignal;
  VisualSignal signal;
  SignalVertexBuffer *buffer;
  Material *boxMaterial, *boxWireMaterial;

  Material * playPosMarkerMaterial;
  
  ControlPotmeter *sliderFishEyeSignalFraction;
  ControlPotmeter *sliderFishEyeLengthScale;

};
#endif


