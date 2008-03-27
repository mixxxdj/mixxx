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
//#include "readerextractbeat.h"
#include "rtthread.h"
#include "visual/visualchannel.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "configobject.h"
#include <QDebug>

Reader::Reader(EngineBuffer * _enginebuffer, QMutex * _pause)
{
    readerExiting = false;
    enginebuffer = _enginebuffer;
    m_dRate = 0.;
    m_pTrack = 0;
    pause = _pause;
    m_pVisualChannel = 0;

    m_iReaderAccess = 0;

    // Allocate reader extract objects
    readerwave = new ReaderExtractWave(this, enginebuffer);

    // Open the track:
    file_srate = 44100;
    file_length = 0;

    m_pTrackEnd = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(enginebuffer->getGroup(),"TrackEnd")));
    m_pButtonCueSet = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(enginebuffer->getGroup(), "cue_set")));
    m_pButtonPlay = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(enginebuffer->getGroup(), "play")));
}

Reader::~Reader()
{
    if (running())
        stop();

    delete readerwave;
    delete m_pTrackEnd;
    delete m_pButtonCueSet;
    delete m_pButtonPlay;
}

void Reader::addVisual(VisualChannel * pVisualChannel)
{
    m_pVisualChannel = pVisualChannel;
    readerwave->addVisual(m_pVisualChannel);
}

void Reader::requestNewTrack(TrackInfoObject * pTrack, bool bStartAtEndPos)
{
    // qDebug() << "request:" << name;

    TrackQueueType * p = new TrackQueueType;
    p->pTrack = pTrack;
    p->bStartAtEndPos = bStartAtEndPos;

    // Put new track request in queue
    trackqueuemutex.lock();
    trackqueue.append(p);
    trackqueuemutex.unlock();

    // Ensure that the reader is not currently awake, befofe waking
    m_qReaderMutex.lock();
    while (m_iReaderAccess>0)
    {
        m_qReaderMutex.unlock();
        sleep(1);
        m_qReaderMutex.lock();
    }
    m_qReaderMutex.unlock();

    // Wakeup reader
    wake();
}

void Reader::requestSeek(double new_playpos)
{
//    qDebug() << "req seek";

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
    readAhead.wakeAll();
}

CSAMPLE * Reader::getBufferWavePtr()
{
    return (CSAMPLE *)readerwave->getBasePtr();
}


long int Reader::getFileLength()
{
    return file_length;
}

int Reader::getFileSrate()
{
    return file_srate;
}

ReaderExtractWave * Reader::getWavePtr()
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
    //qDebug() << "reader set file pos play " << pos << "i";
    readerwave->filepos_play = pos;
}

void Reader::setRate(double dRate)
{
    m_dRate = dRate;
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
// qDebug() << "newtrack, get pause lock";

    // Set pause while loading new track
    pause->lock();


// qDebug() << "newtrack, got pause lock";

    // Get filename
    bool bStartAtEndPos;

    trackqueuemutex.lock();
    if (!trackqueue.isEmpty())
    {
        TrackQueueType * p = trackqueue.first();
        m_pTrack = p->pTrack;
        bStartAtEndPos = p->bStartAtEndPos;
        trackqueue.remove();
        delete p;
    }
    trackqueuemutex.unlock();

    // Exit if no track info was in queue
    if (m_pTrack==0)
    {
        pause->unlock();
        return;
    }

    readerwave->newSource(m_pTrack);

//     qDebug() << "newtrack, new source set";

    // Initialize the new sound source
    file_srate = readerwave->getRate();
    file_length = readerwave->getLength();

    qDebug() << "file length " << file_length << "i";

    // Reset playpos
    if (bStartAtEndPos)
    {
        readerwave->seek(file_length);
        enginebuffer->setNewPlaypos(file_length);
    }
    else {
        enginebuffer->setNewPlaypos(0);

        //Reset the cue point to the beginning of the track:
        
        m_pButtonCueSet->slotSet(1.0);
        //Reset the play button (I'm not sure why this is necessary, but it is...
        //If you don't believe me, uncomment it and notice that you'll have to click
        //play twice to start playback when you load a new track.)
        m_pButtonPlay->slotSet(0.0);
    }

    // Not at track end anymore
    m_pTrackEnd->slotSet(0.);

    //Modified version of Kevin Schaper's patch to fix NEXT mode:
    //(See: https://sourceforge.net/forum/message.php?msg_id=4386494  )
    ControlObject * trackEndMode = ControlObject::getControl(ConfigKey(enginebuffer->getGroup(), "TrackEndMode"));
    //if a track just ended, and the track end mode is set to next, play the track that was just loaded
    if ( (trackEndMode->get() == TRACK_END_MODE_NEXT))
        m_pButtonPlay->slotSet(1.0);

    // Stop pausing process method
    pause->unlock();
}

void Reader::run()
{
    //qDebug() << "Reader running...";
#ifdef __MACX__
    rtThread();
#endif

    while(!readerExiting)
    {
        // Wait for playback if in buffer is filled.
        readAheadMutex.lock();
        readAhead.wait(&readAheadMutex);

        m_qReaderMutex.lock();
        m_iReaderAccess++;
        m_qReaderMutex.unlock();
        // Check if a new track is requested
        trackqueuemutex.lock();
        bool requeststate = trackqueue.isEmpty();
        trackqueuemutex.unlock();
        if (!requeststate)
            newtrack();

// qDebug() << "check seek";

        // Check if a seek is requested
        seekqueuemutex.lock();
        requeststate = seekqueue.isEmpty();
        seekqueuemutex.unlock();
        if (!requeststate)
            seek();

//  qDebug() << "read";

        // Read a new chunk:
        readerwave->getchunk(m_dRate);

        m_qReaderMutex.lock();
        m_iReaderAccess--;
        m_qReaderMutex.unlock();

        readAheadMutex.unlock();
    }
    //qDebug() << "reader stopping";
}

void Reader::stop()
{
    readerExiting = true;
    readAhead.wakeAll();
    wait();
}

void Reader::seek()
{
    double new_playpos = -1.;;

    // Get new play position
    seekqueuemutex.lock();
    if (!seekqueue.isEmpty())
    {
        while (!seekqueue.isEmpty())
        {
            new_playpos = seekqueue.takeFirst();
            //seekqueue.remove(new_playpos);
        }
    }
    seekqueuemutex.unlock();

//    qDebug() << "seek " << new_playpos;

    // Return if queue was empty
    if (new_playpos==-1.)
        return;

    new_playpos = readerwave->seek((long int)new_playpos);
}

