/***************************************************************************
                          visualchannel.cpp  -  description
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

#include "visualchannel.h"
#include "../defs.h"
#include "../readerextract.h"
#include "visualcontroller.h"
#include "visualbuffersignal.h"
#include "visualbuffermarks.h"
#include "visualdisplay.h"

int VisualChannel::siChannelTotal = 0;

/**
 * Default Consructor.
 */

VisualChannel::VisualChannel(ControlPotmeter *pPlaypos, VisualController *pVisualController, const char *_group)
{
    m_iChannelNo = siChannelTotal;
    siChannelTotal++;
    group = (char *)_group;
    
    m_pPlaypos = pPlaypos;
    m_pVisualController = pVisualController;
    m_qlListBuffer.setAutoDelete(true);
    m_qlListDisplay.setAutoDelete(true);
}

VisualChannel::~VisualChannel()
{
    siChannelTotal--;
}

void VisualChannel::zoom(int id)
{
    VisualDisplay *d;
    for (d = m_qlListDisplay.first(); d; d = m_qlListDisplay.next())
        if (id==d->getId())
            d->zoom();
}

VisualBuffer *VisualChannel::add(ReaderExtract *pReaderExtract)
{
    VisualBuffer *b;

    // Determine type.... A hack, yes!
    if (pReaderExtract->getVisualDataType()=="signal")
    {
        // Construct a new buffer
        b = new VisualBufferSignal(pReaderExtract, m_pPlaypos);
    }
    else if (pReaderExtract->getVisualDataType()=="marks")
    {
        // Construct a new buffer
        b = new VisualBufferMarks(pReaderExtract, m_pPlaypos, group);
        b->setColorFg(m_fColorBeatR, m_fColorBeatG, m_fColorBeatB);
        b->setColorBg(m_fColorBackR, m_fColorBackG, m_fColorBackB);
    }

    // And a corresponding display. If the display is the first, also draw box
    // ADD GROUP INFO *****************
    VisualDisplay *d;
    if (m_qlListDisplay.isEmpty())
        d = new VisualDisplay(b, pReaderExtract->getVisualDataType(), group, true);
    else
        d = new VisualDisplay(b, pReaderExtract->getVisualDataType(), group, false);

    //
    // Setup position of display
    //

    // Base y pos dependent on number of containers
    d->setBasepos(m_iPosX,0 /*10*(m_qlListDisplay.count())*/, 0);
    d->setLength(length);
    d->setHeight(height);
    d->setColorSignal(m_fColorSignalR, m_fColorSignalG, m_fColorSignalB);
    d->setColorMarker(m_fColorMarkerR, m_fColorMarkerG, m_fColorMarkerB);
    d->setColorBeat(m_fColorBeatR, m_fColorBeatG, m_fColorBeatB);
    d->setColorFisheye(m_fColorFisheyeR, m_fColorFisheyeG, m_fColorFisheyeB);
    
    // Zoom y pos dependent on channel
    if (m_iChannelNo==0)
        d->setZoompos(m_iZoomPosX,20,0);
    else
        d->setZoompos(m_iZoomPosX,-10,0);

    // Append buffer and display to corresponding lists
    m_qlListBuffer.append(b);
    m_qlListDisplay.append(d);

//    if (m_qlListDisplay.count()==1)
//        d->zoom();

    m_pVisualController->add(d);

    return b;
}

void VisualChannel::move(int msec)
{
    VisualDisplay *d;
    for (d = m_qlListDisplay.first(); d; d = m_qlListDisplay.next())
        d->move(msec);
}

void VisualChannel::setPosX(int x)
{
    m_iPosX = x;
}

void VisualChannel::setZoomPosX(int x)
{
    m_iZoomPosX = x;
}

void VisualChannel::setLength(float l)
{
    length = l;
}

void VisualChannel::setHeight(float h)
{
    height = h;
}


void VisualChannel::toggleFishEyeMode()
{
    VisualDisplay *d;
    for ( d = m_qlListDisplay.first(); d; d = m_qlListDisplay.next() )
        d->toggleFishEyeMode();
}

void VisualChannel::setColorSignal(float r, float g, float b)
{
    m_fColorSignalR = r;
    m_fColorSignalG = g;
    m_fColorSignalB = b;
}

void VisualChannel::setColorBack(float r, float g, float b)
{
    m_fColorBackR = r;
    m_fColorBackG = g;
    m_fColorBackB = b;
}

void VisualChannel::setColorMarker(float r, float g, float b)
{
    m_fColorMarkerR = r;
    m_fColorMarkerG = g;
    m_fColorMarkerB = b;
}

void VisualChannel::setColorBeat(float r, float g, float b)
{
    m_fColorBeatR = r;
    m_fColorBeatG = g;
    m_fColorBeatB = b;
}

void VisualChannel::setColorFisheye(float r, float g, float b)
{
    m_fColorFisheyeR = r;
    m_fColorFisheyeG = g;
    m_fColorFisheyeB = b;
}

