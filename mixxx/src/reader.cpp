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

#include <qfileinfo.h>
#include "enginebuffer.h"
#include "readerextractwave.h"
#include "soundsource.h"
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif
#ifdef __VISUALS__
  #include "visual/visualchannel.h"
#endif

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
    file = 0;
    file_srate = 44100;
    file_length = 0;
}

Reader::~Reader()
{
    if (running())
        stop();

    delete readerwave;

    if (file != 0)
        delete file;

    delete readAhead;
}

void Reader::addVisual(VisualChannel *pVisualChannel)
{
#ifdef __VISUALS__
    m_pVisualChannel = pVisualChannel;
    readerwave->addVisual(m_pVisualChannel);
#endif
}

void Reader::requestNewTrack(QString name)
{
//    qDebug("request: %s",name->latin1());

    // Put new track request in queue
    trackqueuemutex.lock();
    trackqueue.append(name);
    trackqueuemutex.unlock();
    
    // Wakeup reader
    wake();
}

void Reader::requestSeek(double new_playpos)
{
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

    QString filename("");

    // Get filename
    trackqueuemutex.lock();
    if (!trackqueue.isEmpty())
    {
        TTrackQueue::iterator it = trackqueue.begin();
        filename = (*it);
        trackqueue.remove(it);
    }
    trackqueuemutex.unlock();

    // Exit if no filename was in queue
    if (filename == "")
        return;
        
    // If we are already playing a file, then get rid of the sound source:
    if (file != 0)
    {
        delete file;
        file = 0;
    }
//    qDebug("filename: %s",filename->latin1());

    // Initialize the new sound source
    enginelock.lock();
    file_srate = 44100;
    file_length = 0;
    f_dCuePoint = 0;

    if (filename != 0)
    {
        // Check if filename is valid
        QFileInfo finfo(filename);
        if (finfo.exists())
        {
            if (finfo.extension(false).upper() == "WAV")
#ifdef __UNIX__
                file = new SoundSourceAudioFile(filename);
#endif
#ifdef __WIN__
                file = new SoundSourceSndFile(filename);
#endif
            else if (finfo.extension(false).upper() == "MP3")                    
                file = new SoundSourceMp3(filename);
            else if (finfo.extension(false).upper() == "OGG")
                file = new SoundSourceOggVorbis(filename);

            file_srate = file->getSrate();
            file_length = file->length();
        }
    }
    else
#ifdef __UNIX__
        file = new SoundSourceAudioFile( QString("/dev/null") );
#endif
#ifdef __WIN__
        file = new SoundSourceSndFile( QString("/dev/null") );
#endif
    enginelock.unlock();
    if (file==0)
        qFatal("Error opening %s", filename.latin1());

    readerwave->setSoundSource(file);

    // ...and read one chunk to get started:
    readerwave->getchunk(1.);


    // Reset playpos
    enginebuffer->setNewPlaypos(0.);    

    // Stop pausing process method
    pause->unlock();
}

void Reader::run()
{
    //qDebug("Reader running...");

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
        readerwave->getchunk(rate->read());
    }
    //qDebug("reader stopping");
}

void Reader::stop()
{
    readAhead->wakeAll();

    requestStop.lock();
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

    qDebug("seek %f",new_playpos);
    
    // Return if queue was empty
    if (new_playpos==-1.)
        return;
        
    new_playpos = readerwave->seek((long int)new_playpos);
    wake();

    // Set playpos
    enginebuffer->setNewPlaypos(new_playpos);
}



