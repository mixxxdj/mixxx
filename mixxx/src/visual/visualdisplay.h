/***************************************************************************
                          visualdisplay.h  -  description
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

#ifndef VISUALDISPLAY_H
#define VISUALDISPLAY_H

#include "visualobject.h"
#include "visualdisplaybuffer.h"
#include "visualbuffer.h"
#include "visualbox.h"

class ControlPotmeter;
class Material;

const float baselength = 350.;
const float baseheight = 90.;
const float basedepth = 5.;
const float zoomlength = 100.;
const float zoomheight = 15.;
const float zoomdepth = 5.;

/**
 * A Visual Display. Usually a fisheye signal with a box.
 */
class VisualDisplay : public VisualObject
{
    Q_OBJECT
public:
    /**
      * Default Consructor.
      */
    VisualDisplay(VisualBuffer *pVisualBuffer, const char *group, bool drawBox=false);
    ~VisualDisplay();
    /** Returns unique id of signal */
    int getId();
    void update(float * signal,int samples);
    /**
      * Specialized Drawing Routine.
      * This method overrides the oridnary behavior of a visual
      * object, because the VisualDisplay is composed of several
      * smaller visual and/or pickable objects.
      *
      * @param mode     The render mode.
      */
    void draw(GLenum mode);
    void draw();

    void setBasepos(float x, float y, float z);
    void setZoompos(float x, float y, float z);
    void zoom();
    void move(int msec);
    
public slots:
    /**
      * Toggles Fish Eye Mode between on and off.
      *
      */
    void toggleFishEyeMode();
    /**
      * Set Fish Eye Scale.
      *
      * @param scale    A non zero value indicating how long the fish
      *                 eye box should be stretched. 0 means no fisheye, 1 means full length fish eye
      */
    void setFishEyeLengthScale(FLOAT_TYPE scale);
    /** Controls the fraction of the signal display in fish eye */
    void setFishEyeSignalFraction(FLOAT_TYPE fraction);
    /**
      * Set Signal Scale
      *
      * @param scale   A non zero value indicating how heigh signals
      *                in the fish eye box should stretched.
      */
    void setSignalScale(float scale);
    /**
      * Set Rotation.
      * Rotations are currently not fully implemented.
      *
      */
    void setRotation(float angle, float rx,float ry,float rz);

protected:

    void setupScene();
    void doLayout();

    void zoom(float ox, float oy, float oz, float length, float height, float width);

    float ox,oy,oz;   ///< Origio of visual signal (from where signal propagates from)
    float angle;      ///< Rotation angle in radians.
    float rx,ry,rz;   ///< Rotation Axe.

    float fishEyeLengthScale;
    float signalScale;
    bool  fishEyeMode;
    float fishEyeSignalFraction;

    float length;     ///< Signal Length.
    float height;     ///< Signal Heigth.
    float depth;

    VisualBox *box, *playPosMarker;

    VisualDisplayBuffer *preSignal, *fishEyeSignal, *postSignal, *signal;
    VisualBuffer *m_pVisualBuffer;
  
    ControlPotmeter *sliderFishEyeSignalFraction;
    ControlPotmeter *sliderFishEyeLengthScale;

    /** Materials */
    static Material dblue, lblue, purple, lgreen;

    /** Unique id of signal */
    int id;
    /** Count total number of instantiated objects. Used to assign unique id's to each signal */
    static int idCount;

    /** Base position */
    float               basex, basey, basez;
    /** Zoom position */
    float               zoomx, zoomy, zoomz;
    /** Destination position and size used when in movement */
    float               destx, desty, destz, destl, desth, destd;
    /** True if container is currently at or moving towards basepos */
    bool                atBasepos;
    /** True if the container is currently moving */
    bool                movement;
    /** True if box has to be drawn in fish eye mode */
    bool m_bDrawBox;
 
};
#endif


