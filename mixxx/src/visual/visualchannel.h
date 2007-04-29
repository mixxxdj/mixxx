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
#include "visualdisplay.h"

class VisualBuffer;
class ReaderExtract;
class VisualController;
class ControlPotmeter;
class EngineBuffer;

/**
 * A Visual Channel
 * This class keeps track of the visual classes associated with one channel
 */
class VisualChannel : public QObject
{
public:
    VisualChannel(VisualController *pVisualController, const char *_group);
    ~VisualChannel();
    void setupBuffer();
    /** Add a ReaderExtract buffer to object, and construct a corresponding display. A
      * pointer to the constructed VisualBuffer is returned */
    VisualBuffer *add(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, QString qType="");
    void setColorSignal(float r, float g, float b);
    void setColorHfc(float r, float g, float b);
    void setColorCue(float r, float g, float b);
    void setColorMarker(float r, float g, float b);
    void setColorBack(float r, float g, float b);
    void setColorBeat(float r, float g, float b);
    void setColorFisheye(float r, float g, float b);

    /** Toggle fish eye mode on all connected VisualDisplays */
    void toggleFishEyeMode();

	void resetColors();
private:
    VisualController        *m_pVisualController;
    /** List of buffers associated with this channel */
    QPtrList<VisualBuffer>  m_qlListBuffer;
    /** List of displays associated with this channel */
    QPtrList<VisualDisplay>  m_qlListDisplay;
    /** channel number */
    int m_iChannelNo;
    /** Total number of channels */
    static int siChannelTotal;
    /** Control group */
    char *group;
    /** Colors */
    float m_fColorSignalR, m_fColorSignalG, m_fColorSignalB;
    float m_fColorHfcR, m_fColorHfcG, m_fColorHfcB;
    float m_fColorCueR, m_fColorCueG, m_fColorCueB;
    float m_fColorMarkerR, m_fColorMarkerG, m_fColorMarkerB;
    float m_fColorBeatR, m_fColorBeatG, m_fColorBeatB;
    float m_fColorBackR, m_fColorBackG, m_fColorBackB;
    float m_fColorFisheyeR, m_fColorFisheyeG, m_fColorFisheyeB;

};
#endif
