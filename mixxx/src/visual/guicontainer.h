/***************************************************************************
                          guicontainer.h  -  description
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
 
#ifndef GUICONTAINER_H
#define GUICONTAINER_H

#include <qobject.h>
#include "material.h"
#include "light.h"

class FastVertexArray;
class SignalVertexBuffer;
class GUISignal;
class EngineBuffer;


const float baselength = 25.;
const float baseheight = 5.;
const float basedepth = 1.;
const float zoomlength = 100.;
const float zoomheight = 15.;
const float zoomdepth = 5.;

/*
const float baselength = 25.;
const float baseheight = 5.;
const float basedepth = 1.;
const float zoomlength = 25.;
const float zoomheight = 5.;
const float zoomdepth = 1.;
*/

/**
 * A GUI Container
 * This class keeps track of GUISignal, VisualSignalBuffer and vertex buffer
 * associated to plot a signal.
 *
 *
 */
class GUIContainer : public QObject
{
public:
    GUIContainer(FastVertexArray *vertex, EngineBuffer *engineBuffer);

    GUIContainer *getContainer(int id);
    GUISignal *getSignal();
    SignalVertexBuffer *getBuffer();

    void setBasepos(float x, float y, float z);
    void setZoompos(float x, float y, float z);
    void zoom();

    void move(int msec);
private:
    void setupScene();
    void zoom(float ox, float oy, float oz, float length, float height, float width);

    static int          idCount;
    int                 id;
    SignalVertexBuffer *buffer;
    FastVertexArray    *vertex;
    GUISignal          *signal;

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

    static Light mylight;
    static Material dblue, lblue, purple, lgreen;
};
#endif
