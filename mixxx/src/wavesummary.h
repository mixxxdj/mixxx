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
#include <q3ptrqueue.h>
#include "defs.h"
#include "configobject.h"

class TrackInfoObject;
class ControlObjectThread;

/**
  * Class for generating waveform summaries
  *
  *@author Tue Haste Andersen
  */

#ifndef WAVESUMMARYCONSTANTS
const int kiBlockSize = 2048;
const int kiBeatBlockNo = 1000;
const int kiBeatBins = 100;
const int kiSummaryBufferSize = 2100;
const float kfFeatureStepSize = 0.01f;
#define WAVESUMMARYCONSTANTS
#endif

class SoundSourceProxy;

class WaveSummary : public QThread
{
public:
    WaveSummary(ConfigObject<ConfigValue> *_config);
    ~WaveSummary();
    /** Puts an TrackInfoObject into the queue of summary generation. Thread safe, blocking. */
    void enqueue(TrackInfoObject *pTrackInfoObject);
	void extractBeat(TrackInfoObject *pTrackInfoObject);

private:
    /** Main thread loop */
    void run();
    /** Stop the thread */
    void stop();
    /** Generate a visual waveform for the given track */
    void visualWaveformGen(TrackInfoObject *pTrackInfoObject, SoundSourceProxy *pSoundSource);
    /** Generate a waveform summary for the given track */
    void waveformSummaryGen(TrackInfoObject *pTrackInfoObject, SoundSourceProxy *pSoundSource);
    /** Queue holding files to generate a summary for */
    Q3PtrQueue<TrackInfoObject> m_qQueue;
    /** Mutex controlling access to m_qQueue */
    QMutex m_qMutex;
    /** Wait condition */
    QWaitCondition m_qWait;
    /** Pointer to config object **/
    ConfigObject<ConfigValue> *m_Config;
    
    ControlObjectThread *m_pControlVisualResample;

    bool m_bShouldExit;


};

#endif
