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

class SoundBuffer;
class SoundSource;
class EngineBuffer;

/**
  *@author Tue & Ken Haste Andersen
  */

class Reader: public QThread
{
public: 
    Reader(EngineBuffer *_enginebuffer, Monitor *_rate, QMutex *_pause);
    ~Reader();

    /** Request new track to be loaded. This method is thread safe, but may block */
    void requestNewTrack(QString name);
    /** Request seek. This method is thread safe, but may block */
    void requestSeek(double new_playpos);
    /** Wake up reader thread. Thread safe, non-blocking */
    void wake();
    /** Get wave buffer pointer. This address is used by EngineBuffer. The method is
      * not thread safe and should be called before the reader thread is started */
    CSAMPLE *getBufferWavePtr();
    /** Should this really be possible??? */
    SoundBuffer *getSoundBuffer();

    /** Mutex controlling access to file_srate, file_length along with filepos_start and
      * filepos_end from SoundBuffer. These variables are shared between the reader and the
      * player (engine) thread */
    QMutex enginelock;
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
private:
    void run();
    void stop();
    void newtrack();
    /** Seek to a new position. */
    void seek();

    SoundSource *file;
    /** Pointer to rate monitor allocated and written in EngineBuffer. */
    Monitor *rate;
    /** Pointer to mutex allocated in EngineBuffer, controlling rendering in EngineBuffer::process.
      * While holding this mutex, the EngineBuffer::process will silence, not reading the sound buffer */
    QMutex *pause;
      
    SoundBuffer *soundbuffer;
    EngineBuffer *enginebuffer;
    
    QMutex requestStop;
    QWaitCondition *readAhead;

    /** Track queue used in communication with reader from other threads */
    typedef QValueList<QString> TTrackQueue;
    TTrackQueue trackqueue;
    /** Seek queue used in communication with reader from other threads */
    typedef QValueList<double> TSeekQueue;
    TSeekQueue seekqueue;
    /** Mutex used when accessing queues */
    QMutex trackqueuemutex, seekqueuemutex;
    /** Local copy of file sample rate */
    int file_srate;
    /** Local copy of file length */
    int file_length;
};

#endif
