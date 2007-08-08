/***************************************************************************
bpmdetector.h  -  The bpm detection queue
-------------------
begin                : Sat, Aug 4., 2007
copyright            : (C) 2007 by Micah Lee
email                : snipexv@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef BPMDETECTOR_H
#define BPMDETECTOR_H

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <q3ptrqueue.h>
#include "defs.h"
#include "configobject.h"

class TrackInfoObject;
class ControlObjectThread;

#ifndef WAVESUMMARYCONSTANTS
const int kiBlockSize = 2048;
const int kiBeatBlockNo = 1000;
const int kiBeatBins = 100;
const int kiSummaryBufferSize = 2100;
const float kfFeatureStepSize = 0.01f;
#define WAVESUMMARYCONSTANTS
#endif

class WindowKaiser;
class EngineSpectralFwd;

/**
  * Class for detecting the BPM for a TrackInfoObject
  *
  *@author Micah Lee
  */

class BpmDetector : public QThread
{
public:
    BpmDetector(ConfigObject<ConfigValue> *_config);
    ~BpmDetector();
    /** Puts an TrackInfoObject into the queue of BPM detection. Thread safe, blocking. */
    void enqueue(TrackInfoObject *pTrackInfoObject);

protected:
    /** Main thread loop */
    void run();

    /** Queue holding files to generate a summary for */
    Q3PtrQueue<TrackInfoObject> m_qQueue;
    /** Mutex controlling access to m_qQueue */
    QMutex m_qMutex;
    /** Wait condition */
    QWaitCondition m_qWait;
    QMutex m_qWaitMutex;
    /** Pointer to window and windowed samples of signal */
    WindowKaiser *window;
    /** Pointer to samples containing one windowed frame of samples */
    CSAMPLE *windowedSamples;
    /** Pointer to array containing window */
    CSAMPLE *windowPtr;
    EngineSpectralFwd *m_pEngineSpectralFwd;	
	/** Pointer to config object **/
	ConfigObject<ConfigValue> *m_Config;
};

#endif
