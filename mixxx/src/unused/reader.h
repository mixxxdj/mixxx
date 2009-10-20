/***************************************************************************
                          reader.h  -  description
                             -------------------
    begin                : Thu Mar 13 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef READER_H
#define READER_H

#include <QtCore>
#include "configobject.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "monitor.h"

class ReaderExtractWave;
class ReaderExtractBeat;
class EngineBuffer;
class ControlObjectThread;

/**
  * The Reader class is a thread taking care of reading and buffering waveform data from external sources.
  * The source can be any SoundSource derived object. The Reader is waked up by signals from the EngineBuffer
  * whenever data is needed. The Reader calls SoundSource to read the data, followed by calls to a number of
  * ReaderExtract objects to perform feature extraction from the waveform data.
  *
  *@author Tue Haste Andersen
  */

class Reader: public QThread
{
    Q_OBJECT 
public:
    Reader(EngineBuffer *_enginebuffer, QMutex *_pause, ConfigObject<ConfigValue> * _config);
    ~Reader();


    /** Request new track to be loaded. This method is thread safe, but may block */
    void requestNewTrack(TrackInfoObject *pTrack, bool bStartAtEndPos=false);
    /** Request seek. This method is thread safe, but may block */
    void requestSeek(double new_playpos);
    /** Wake up reader thread. Thread safe, non-blocking */
    void wake();
    /** Get wave buffer pointer. This address is used by EngineBuffer. The method is
      * not thread safe and should be called before the reader thread is started */
    CSAMPLE *getBufferWavePtr();
    /** Get pointer to wave extraction object */
    ReaderExtractWave *getWavePtr();
    /** Tries to lock mutex controlling access to file_srate, file_length and filepos_start. Non-blocking */
    bool tryLock();
    /** Lock mutex controlling access to file_srate, file_length and filepos_start. Blocking. */
    void lock();
    /** Unlock mutex controlling access to file_srate, file_length and filepos_start */
    void unlock();
    /** Returns file length. This method must only be called when holding the enginelock
      * mutex */
    long int getFileLength();
    /** Returns file sample rate. This method must only be called when holding the enginelock
      * mutex */
    int getFileSrate();
    /** Returns file length. This method must only be called when holding the enginelock
      * mutex */
    long int getFileposStart();
    /** Returns file sample rate. This method must only be called when holding the enginelock
      * mutex */
    long int getFileposEnd();
    /** Set the file play position. This method must only be called when holding the enginelock
      * mutex */
    void setFileposPlay(long int);
    /** Set rate. This method must only be called when holding the enginelock
      * mutex */
    void setRate(double dRate);

signals:
    void finishedLoading(TrackInfoObject* track, bool bStartAtEndPos);

private:
    /** Config file accessor */
    ConfigObject<ConfigValue>* m_pConfig;
    /** Main loop of the thread */
    void run();
    /** Stop thread */
    void stop();
    /** Load new track */
    void newtrack();
    /** Seek to a new position. */
    void seek();
    /** Mutex controlling access to file_srate, file_length along with filepos_start and
      * filepos_end from ReaderBuffer. These variables are shared between the reader and the
      * player (engine) thread */
    QMutex enginelock;
    /** Rate. Updated from EngineBuffer. */
    double m_dRate;
    /** Pointer to mutex allocated in EngineBuffer, controlling rendering in EngineBuffer::process.
      * While holding this mutex, the EngineBuffer::process will silence, not reading the sound buffer */
    QMutex *pause;
    /** Pointer to ReaderExtractWave object */
    ReaderExtractWave *readerwave;
    /** Pointer to EngineBuffer */
    EngineBuffer *enginebuffer;
    /** Mutex used in termination of the thread */
	// FIXME: This mutex doesn't work on windows?!?!? using a boolean instead for now
    QMutex requestStop;
	bool readerExiting;
    /** Mutex used to sync this thread (reader) with other threads */
    QMutex m_qReaderMutex;
    /** Variable used with readerMutex to sync thread. Must only be modified when holding readerMutex */
    int m_iReaderAccess;
    /** Lets us reset the cue point when a new track loads */
    ControlObjectThread* m_pButtonCueSet;
    /** Lets us start playing a new track in NEXT mode */
    ControlObjectThread* m_pButtonPlay;
    /** Lets us do something at the end of a track */ 
    ControlObjectThread *m_pTrackEnd;
    /** Wait condition to make thread sleep when not needed */
    QWaitCondition readAhead;
    QMutex readAheadMutex;
    typedef struct TrackQueueType {
        TrackInfoObject *pTrack;
        bool bStartAtEndPos;
    } TrackQueueType;
    typedef Q3PtrList<TrackQueueType> TTrackQueue;
    /** Track queue used in communication with reader from other threads */
    TTrackQueue trackqueue;
    typedef QList<double> TSeekQueue;
    /** Seek queue used in communication with reader from other threads */
    TSeekQueue seekqueue;
    /** Current loaded track */
    TrackInfoObject *m_pTrack;
    /** Mutex used when accessing queues */
    QMutex trackqueuemutex, seekqueuemutex;
    /** Local copy of file sample rate */
    int file_srate;
    /** Local copy of file length */
    long int file_length;
};

#endif
