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

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qvaluelist.h>
#include "defs.h"
#include "monitor.h"

class ReaderExtractWave;
class ReaderExtractBeat;              
class SoundSource;
class EngineBuffer;
class GUIChannel;

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
public: 
    Reader(EngineBuffer *_enginebuffer, Monitor *_rate, QMutex *_pause);
    ~Reader();

    void addVisual(GUIChannel *guichannel);
    /** Request new track to be loaded. This method is thread safe, but may block */
    void requestNewTrack(QString name);
    /** Request seek. This method is thread safe, but may block */
    void requestSeek(double new_playpos);
    /** Wake up reader thread. Thread safe, non-blocking */
    void wake();
    /** Get wave buffer pointer. This address is used by EngineBuffer. The method is
      * not thread safe and should be called before the reader thread is started */
    CSAMPLE *getBufferWavePtr();
    /** Get pointer to beat extraction object */
    ReaderExtractBeat *getBeatPtr();
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
    int getFileLength();
    /** Returns file sample rate. This method must only be called when holding the enginelock
      * mutex */
    int getFileSrate();
    /** Returns file length. This method must only be called when holding the enginelock
      * mutex */
    long int getFileposStart();
    /** Returns file sample rate. This method must only be called when holding the enginelock
      * mutex */
    long int getFileposEnd();
    /** Cue point */
    double f_dCuePoint;
    
private:
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
    /** Pointer to sound source */
    SoundSource *file;
    /** Pointer to rate monitor allocated and written in EngineBuffer. */
    Monitor *rate;
    /** Pointer to mutex allocated in EngineBuffer, controlling rendering in EngineBuffer::process.
      * While holding this mutex, the EngineBuffer::process will silence, not reading the sound buffer */
    QMutex *pause;
    /** Pointer to ReaderExtractWave object */  
    ReaderExtractWave *readerwave;
    /** Pointer to EngineBuffer */
    EngineBuffer *enginebuffer;
    /** Mutex used in termination of the thread */
    QMutex requestStop;
    /** Wait condition to make thread sleep when not needed */
    QWaitCondition *readAhead;
    typedef QValueList<QString> TTrackQueue;
    /** Track queue used in communication with reader from other threads */
    TTrackQueue trackqueue;
    typedef QValueList<double> TSeekQueue;
    /** Seek queue used in communication with reader from other threads */
    TSeekQueue seekqueue;
    /** Mutex used when accessing queues */
    QMutex trackqueuemutex, seekqueuemutex;
    /** Local copy of file sample rate */
    int file_srate;
    /** Local copy of file length */
    int file_length;
    /** Pointer to GUIChannel */
    GUIChannel *guichannel;
};

#endif
