/***************************************************************************
                          wavesummaryevent.h  -  description
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

#ifndef WAVESUMMARYEVENT_H
#define WAVESUMMARYEVENT_H

#include <qevent.h>
#include <qmemarray.h>
#include <qvaluelist.h>

/**
  *@author Tue Haste Andersen
  *
  * Event used in communication from WaveSummary to TrackInfoObject
  *
  */

class WaveSummaryEvent : public QCustomEvent 
{
public:
    WaveSummaryEvent(QMemArray<char> *pWave, QValueList<long> *pSegmentation);
    ~WaveSummaryEvent();
    QMemArray<char> *wave() const;
    QValueList<long> *segmentation() const;

private:
    QMemArray<char> *m_pWave;
    QValueList<long> *m_pSegmentation;
};

#endif
