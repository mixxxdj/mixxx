#ifndef GUICONTAINER_H
#define GUICONTAINER_H

#include <qobject.h>
#include "material.h"
#include "light.h"

class CFastVertexArray;
class CSignalVertexBuffer;
class CGUISignal;
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
    GUIContainer(CFastVertexArray *vertex, EngineBuffer *engineBuffer);

    GUIContainer *getContainer(int id);
    CGUISignal *getSignal();
    CSignalVertexBuffer *getBuffer();

    void setBasepos(float x, float y, float z);
    void setZoompos(float x, float y, float z);
    void zoom();

    void move(int msec);
private:
    void setupScene();
    void zoom(float ox, float oy, float oz, float length, float height, float width);

    static int          idCount;
    int                 id;
    CSignalVertexBuffer *buffer;
    CFastVertexArray    *vertex;
    CGUISignal          *signal;

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

    static CLight mylight;
    static CMaterial dblue, lblue, purple, lgreen;
};
#endif
