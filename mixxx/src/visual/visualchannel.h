/***************************************************************************
                          visualchannel.h  -  description
                             -------------------
    copyright            : (C) 2003 by Tue and Ken Haste Andersen and Kenny
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
 
#ifndef VISUALCHANNEL_H
#define VISUALCHANNEL_H

#include <qobject.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qevent.h>

#include "visualbuffer.h"
#include "visualdisplay.h"

class ReaderExtract;
class VisualController;
class ControlPotmeter;

/**
 * A Visual Channel
 * This class keeps track of the visual classes associated with one channel
 */
class VisualChannel : public QObject
{
public:
    VisualChannel(ControlPotmeter *pPlaypos, VisualController *pVisualController, const char *_group);
    ~VisualChannel();
    /** Zoom/unzoom the signal of id */
    void zoom(int id);
    /** Add a ReaderExtract buffer to object, and construct a corresponding display. A
      * pointer to the constructed VisualBuffer is returned */
    VisualBuffer *add(ReaderExtract *pReaderExtract);
    void move(int msec);
    void setPosX(int x);
    void setZoomPosX(int x);
    void setLength(float l);
    void setHeight(float h);
    void setColorSignal(float r, float g, float b);
    void setColorHfc(float r, float g, float b);
    void setColorMarker(float r, float g, float b);
    void setColorBack(float r, float g, float b);
    void setColorBeat(float r, float g, float b);
    void setColorFisheye(float r, float g, float b);

    /** Toggle fish eye mode on all connected VisualDisplays */
    void toggleFishEyeMode();
private:
    VisualController        *m_pVisualController;
    ControlPotmeter         *m_pPlaypos;
    /** List of buffers associated with this channel */
    QPtrList<VisualBuffer>  m_qlListBuffer;
    /** List of displays associated with this channel */
    QPtrList<VisualDisplay>  m_qlListDisplay;
    /** channel number */
    int m_iChannelNo;
    /** Total number of channels */
    static int siChannelTotal;
    /** X position of this channel */
    int m_iPosX, m_iZoomPosX;
    /** Length and height of gl widget */
    float length, height;
    /** Control group */
    char *group;
    /** Colors */
    float m_fColorSignalR, m_fColorSignalG, m_fColorSignalB;
    float m_fColorHfcR, m_fColorHfcG, m_fColorHfcB;
    float m_fColorMarkerR, m_fColorMarkerG, m_fColorMarkerB;
    float m_fColorBeatR, m_fColorBeatG, m_fColorBeatB;
    float m_fColorBackR, m_fColorBackG, m_fColorBackB;
    float m_fColorFisheyeR, m_fColorFisheyeG, m_fColorFisheyeB;

};
#endif
