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
#include "material.h"

class ControlPotmeter;

// These variables, together with the aspect ratio and view point set in 
// VisualController controls the screen size of the displayed signals.
const float kfVisualDisplayLength = 35.f;
const float kfVisualDisplayHeight = 9.5f;
const float kfVisualDisplayDepth = 0.1f;

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
    VisualDisplay(VisualBuffer *pVisualBuffer, const char *type, const char *group, bool drawBox=false);
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

    void setColorSignal(float r, float g, float b);
    void setColorHfc(float r, float g, float b);
    void setColorBeat(float r, float g, float b);
    void setColorMarker(float r, float g, float b);
    void setColorFisheye(float r, float g, float b);

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
    void setFishEyeLengthScale(double scale);
    /** Controls the fraction of the signal display in fish eye */
    void setFishEyeSignalFraction(double fraction);
    /**
      * Set Signal Scale
      *
      * @param scale   A non zero value indicating how heigh signals
      *                in the fish eye box should stretched.
      */
    void setSignalScaleHeight(double scale);
    void setSignalScaleLength(double scale);
    /** Set Rotation. Rotations are currently not fully implemented. */
    void setRotation(float angle, float rx,float ry,float rz);

protected:
    void doLayout();
    
    /** Origio of visual signal (from where signal propagates from) */
    float ox,oy,oz;
    /** Rotation angle in radians */
    float angle;
    /** Rotation Axe */
    float rx,ry,rz;

    float fishEyeLengthScale;
    float signalScaleHeight;
    float signalScaleLength;
    bool  fishEyeMode;
    float fishEyeSignalFraction;


    VisualBox *box, *playPosMarker;

    VisualDisplayBuffer *preSignal, *fishEyeSignal, *postSignal, *signal;
    VisualBuffer *m_pVisualBuffer;

    ControlPotmeter *controlScaleLength;

    /** Materials */
    Material m_materialSignal, m_materialHfc, m_materialMarker, m_materialBeat, m_materialFisheye;

    /** Unique id of signal */
    int id;
    /** Count total number of instantiated objects. Used to assign unique id's to each signal */
    static int idCount;

    /** True if box has to be drawn in fish eye mode */
    bool m_bDrawBox;

};
#endif


