/***************************************************************************
                          wavesummary.h  -  description
                             -------------------
    begin                : Wed Oct 13 2004
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

#ifndef WAVESUMMARY_H
#define WAVESUMMARY_H

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qptrqueue.h>

class TrackInfoObject;
class ControlObjectThread;

/**
  * Class for generating waveform summaries
  *
  *@author Tue Haste Andersen
  */

const int kiBlockSize = 2048;
const int kiSummaryBufferSize = 300;
const float kfFeatureStepSize = 0.01;

class WaveSummary : public QThread
{
public:
    WaveSummary();
    ~WaveSummary();
    /** Puts an TrackInfoObject into the queue of summary generation. Thread safe, blocking. */
    void enqueue(TrackInfoObject *pTrackInfoObject);

protected:
    /** Main thread loop */
    void run();

    /** Queue holding files to generate a summary for */
    QPtrQueue<TrackInfoObject> m_qQueue;
    /** Mutex controlling access to m_qQueue */
    QMutex m_qMutex;
    /** Wait condition */
    QWaitCondition m_qWait;
};

#endif
