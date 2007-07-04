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
#include "visualcontroller.h"
#include "visualbuffersignal.h"
#include "visualbuffersignalhfc.h"
#include "visualbuffertemporal.h"
#include "visualbuffermarks.h"
#include "visualdisplay.h"
#include "../readerextract.h"
#include "../defs.h"

int VisualChannel::siChannelTotal = 0;

/**
 * Default Consructor.
 */

VisualChannel::VisualChannel(VisualController *pVisualController, const char *_group)
{
    m_iChannelNo = siChannelTotal;
    siChannelTotal++;
    group = (char *)_group;

    m_pVisualController = pVisualController;
    m_qlListBuffer.setAutoDelete(true);
    m_qlListDisplay.setAutoDelete(true);
}

VisualChannel::~VisualChannel()
{
    siChannelTotal--;
}

void VisualChannel::setupBuffer()
{
    // Call each channel associated
//    VisualBuffer *b;
//    for (b=m_qlListBuffer.first(); b; b=m_qlListBuffer.next())
//        b->slotSetupBuffer();
}

VisualBuffer *VisualChannel::add(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, QString qType)
{
    VisualBuffer *b = 0;

    // Determine type.... A hack, yes!
    if (!pReaderExtract)
    {
        // Temporal
        b = new VisualBufferTemporal(pEngineBuffer, group);
    }
    else if (qType=="signal")
    {
        // Construct a new buffer
        b = new VisualBufferSignal(pReaderExtract, pEngineBuffer, group);
    }
    else if (qType=="hfc")
    {
        // Construct a new buffer
        b = new VisualBufferSignalHFC(pReaderExtract, pEngineBuffer, group);
    }
    else if (qType=="marks")
    {
        // Construct a new buffer
        b = new VisualBufferMarks(pReaderExtract, pEngineBuffer, group);
        b->setColorFg(m_fColorBeatR, m_fColorBeatG, m_fColorBeatB);
        b->setColorBg(m_fColorBackR, m_fColorBackG, m_fColorBackB);
    }

    Q_ASSERT(b);

    // And a corresponding display. If the display is the first, also draw box
    // ADD GROUP INFO *****************
    VisualDisplay *d;
    if (m_qlListDisplay.isEmpty())
	{
		if (pReaderExtract)
			d = new VisualDisplay(b, qType, group, true);
		else
			d = new VisualDisplay(b, "temporal", group, true);
	}
	else
    {
		if (pReaderExtract)
			d = new VisualDisplay(b, qType, group, false);
		else
			d = new VisualDisplay(b, "temporal", group, false);
	}

    //
    // Setup position of display
    //

    // Base y pos dependent on number of containers
//     d->setBasepos(m_iPosX,0 /*10*(m_qlListDisplay.count())*/, 0);
//     d->setLength(length);
//     d->setHeight(height);
    d->setColorSignal(m_fColorSignalR, m_fColorSignalG, m_fColorSignalB);
    d->setColorHfc(m_fColorHfcR, m_fColorHfcG, m_fColorHfcB);
    d->setColorCue(m_fColorCueR, m_fColorCueG, m_fColorCueB);
    d->setColorMarker(m_fColorMarkerR, m_fColorMarkerG, m_fColorMarkerB);
    d->setColorBeat(m_fColorBeatR, m_fColorBeatG, m_fColorBeatB);
    d->setColorFisheye(m_fColorFisheyeR, m_fColorFisheyeG, m_fColorFisheyeB);

    // Zoom y pos dependent on channel
//     if (m_iChannelNo==0)
//         d->setZoompos(m_iZoomPosX,20,0);
//     else
//         d->setZoompos(m_iZoomPosX,-10,0);

    // Append buffer and display to corresponding lists
    m_qlListBuffer.append(b);
    m_qlListDisplay.append(d);

//    if (m_qlListDisplay.count()==1)
//        d->zoom();

    m_pVisualController->add(d);

    return b;
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

void VisualChannel::setColorHfc(float r, float g, float b)
{
    m_fColorHfcR = r;
    m_fColorHfcG = g;
    m_fColorHfcB = b;
}

void VisualChannel::setColorCue(float r, float g, float b)
{
    m_fColorCueR = r;
    m_fColorCueG = g;
    m_fColorCueB = b;
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

void VisualChannel::resetColors() {

	VisualBuffer* vb;
	for (vb = m_qlListBuffer.first(); vb; vb = m_qlListBuffer.next()) {
		vb->setColorFg(m_fColorBeatR, m_fColorBeatG, m_fColorBeatB);
        vb->setColorBg(m_fColorBackR, m_fColorBackG, m_fColorBackB);
	}

	VisualDisplay* vd;
	for (vd = m_qlListDisplay.first(); vd; vd = m_qlListDisplay.next()) {
		vd->setColorSignal(m_fColorSignalR, m_fColorSignalG, m_fColorSignalB);
		vd->setColorHfc(m_fColorHfcR, m_fColorHfcG, m_fColorHfcB);
		vd->setColorCue(m_fColorCueR, m_fColorCueG, m_fColorCueB);
		vd->setColorMarker(m_fColorMarkerR, m_fColorMarkerG, m_fColorMarkerB);
		vd->setColorBeat(m_fColorBeatR, m_fColorBeatG, m_fColorBeatB);
		vd->setColorFisheye(m_fColorFisheyeR, m_fColorFisheyeG, m_fColorFisheyeB);
	}
}

