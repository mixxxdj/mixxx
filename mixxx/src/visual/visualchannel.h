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

class ReaderExtract;
class VisualController;
class VisualBuffer;
class VisualDisplay;
class ControlPotmeter;

/**
 * A Visual Channel
 * This class keeps track of the visual classes associated with one channel
 */
class VisualChannel : public QObject
{
public:
    VisualChannel(ControlPotmeter *pPlaypos, VisualController *pVisualController);
    ~VisualChannel();
    bool eventFilter(QObject *o, QEvent *e);
    /** Zoom/unzoom the signal of id */
    void zoom(int id);
    /** Add a ReaderExtract buffer to object, and construct a corresponding display */
    void add(ReaderExtract *pReaderExtract);
    void move(int msec);
    void setPosX(int x);
    void setZoomPosX(int x);    
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
};
#endif
