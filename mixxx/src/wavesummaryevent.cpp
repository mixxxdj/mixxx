/***************************************************************************
                          wavesummaryevent.cpp  -  description
                             -------------------
    begin                : Tue Oct 19 2004
    copyright            : (C) 2004 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wavesummaryevent.h"

WaveSummaryEvent::WaveSummaryEvent(QMemArray<char> *pWave, QValueList<int> *pSegmentation) : QCustomEvent(10001), m_pWave(pWave), m_pSegmentation(pSegmentation)
{
}

WaveSummaryEvent::~WaveSummaryEvent()
{
}

QMemArray<char> *WaveSummaryEvent::wave() const
{
    return m_pWave;
}

QValueList<int> *WaveSummaryEvent::segmentation() const
{
    return m_pSegmentation;
}

