/***************************************************************************
                          reader.cpp  -  description
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

#include "reader.h"

#include "enginebuffer.h"
#include "readerextractwave.h"
#include "rtthread.h"
#include "visual/visualchannel.h"

Reader::Reader(EngineBuffer *_enginebuffer, Monitor *_rate, QMutex *_pause)
{
    enginebuffer = _enginebuffer;
    rate = _rate;
    pause = _pause;
    m_pVisualChannel = 0;

    // Allocate reader extract objects
    readerwave = new ReaderExtractWave(this);

    // Allocate semaphore
    readAhead = new QWaitCondition();

    // Open the track:
    file_srate = 44100;
    file_length = 0;
}

Reader::~Reader()
{
    if (running())
        stop();

    delete readerwave;
    delete readAhead;
}

void Reader::addVisual(VisualChannel *pVisualChannel)
{
    m_pVisualChannel = pVisualChannel;
    readerwave->addVisual(m_pVisualChannel);
}

void Reader::requestNewTrack(TrackInfoObject *pTrack)
{
//    qDebug("request: %s",name->latin1());

    // Put new track request in queue
    trackqueuemutex.lock();
    trackqueue.append(pTrack);
    trackqueuemutex.unlock();

    // Wakeup reader
    wake();
}

void Reader::requestSeek(double new_playpos)
{
//    qDebug("req seek");
    
    // Put seek request in queue
    seekqueuemutex.lock();
    seekqueue.append(new_playpos);
    seekqueuemutex.unlock();

    // Wakeup reader
    wake();
}

void Reader::wake()
{
    // Wakeup reader
    readAhead->wakeAll();
}

CSAMPLE *Reader::getBufferWavePtr()
{
    return (CSAMPLE *)readerwave->getBasePtr();
}


int Reader::getFileLength()
{
    return file_length;
}

int Reader::getFileSrate()
{
    return file_srate;
}

ReaderExtractBeat *Reader::getBeatPtr()
{
    return readerwave->getExtractBeat();
}

ReaderExtractWave *Reader::getWavePtr()
{
    return readerwave;
}

long int Reader::getFileposStart()
{
    return readerwave->filepos_start;
}

long int Reader::getFileposEnd()
{
    return readerwave->filepos_end;
}

void Reader::setFileposPlay(long int pos)
{
    readerwave->filepos_play = pos;
}

bool Reader::tryLock()
{
    return enginelock.tryLock();
}

void Reader::lock()
{
    enginelock.lock();
}

void Reader::unlock()
{
    enginelock.unlock();
}

void Reader::newtrack()
{
    // Set pause while loading new track
    pause->lock();

    // Get filename
    TrackInfoObject *pTrack;
    trackqueuemutex.lock();
    if (!trackqueue.isEmpty())
    {
        pTrack = trackqueue.first();
        trackqueue.remove();
    }
    trackqueuemutex.unlock();

    // Exit if no track info was in queue
    if (pTrack==0)
        return;

    readerwave->newSource(pTrack);

    // Initialize the new sound source
    file_srate = readerwave->getRate();
    file_length = readerwave->getLength();
    f_dCuePoint = 0;

    // Reset playpos
    enginebuffer->setNewPlaypos(0.);

    // Stop pausing process method
    pause->unlock();
}

void Reader::run()
{
    //qDebug("Reader running...");
    rtThread();

    double rate_old = 0.;

    while(!requestStop.locked())
    {
        // Wait for playback if in buffer is filled.
        readAhead->wait();

        // Check if a new track is requested
        trackqueuemutex.lock();
        bool requeststate = trackqueue.isEmpty();
        trackqueuemutex.unlock();
        if (!requeststate)
            newtrack();

        // Check if a seek is requested
        seekqueuemutex.lock();
        requeststate = seekqueue.isEmpty();
        seekqueuemutex.unlock();
        if (!requeststate)
            seek();

        // Read a new chunk:
        double temp;
        if (rate->tryRead(&temp))
            rate_old = temp;
        //qDebug("Get chunk %f",rate_old);
        readerwave->getchunk(rate_old);
    }
    //qDebug("reader stopping");
}

void Reader::stop()
{
    requestStop.lock();
    readAhead->wakeAll();
    wait();
    requestStop.unlock();
}

void Reader::seek()
{
    double new_playpos = -1.;;

    // Get new play position
    seekqueuemutex.lock();
    if (!seekqueue.isEmpty())
    {
        TSeekQueue::iterator it = seekqueue.begin();
        new_playpos = (*it);
        seekqueue.remove(it);
    }
    seekqueuemutex.unlock();

//    qDebug("seek %f",new_playpos);

    // Return if queue was empty
    if (new_playpos==-1.)
        return;

    // Set playpos
    enginebuffer->setNewPlaypos(new_playpos);

    new_playpos = readerwave->seek((long int)new_playpos);
    wake();

}



