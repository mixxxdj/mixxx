/***************************************************************************
                          readerextractwave.cpp  -  description
                             -------------------
    begin                : Thu Feb 6 2003
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
#include "readerextractwave.h"
#include "readerextractfft.h"
#include "readerextracthfc.h"
#include "readerextractbeat.h"
#include "readerevent.h"
#include "visual/visualchannel.h"
#include "visual/visualbuffer.h"
#include "soundsource.h"
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif
#include <qfileinfo.h>

ReaderExtractWave::ReaderExtractWave(Reader *pReader) : ReaderExtract(0, "signal")
{
    m_pReader = pReader;
    
    // Allocate temporary buffer
    temp = new SAMPLE[READCHUNKSIZE*2];

    // Allocate read_buffer
    read_buffer = new CSAMPLE[READBUFFERSIZE];
    for (unsigned i=0; i<READBUFFERSIZE; ++i)
        read_buffer[i] = 0.;

    // Initialize position in read buffer
    m_pReader->lock();
    filepos_start = 0;
    filepos_end = 0;
    filepos_play = 0;
    m_pReader->unlock();
    bufferpos_start = 0;
    bufferpos_end = 0;

    file = 0;

    // Initialize extractor objects
    readerfft = 0;
    readerhfc = 0;
    readerbeat = 0;
#ifdef EXTRACT
    readerfft  = new ReaderExtractFFT((ReaderExtract *)this, WINDOWSIZE, STEPSIZE);
    readerhfc  = new ReaderExtractHFC((ReaderExtract *)readerfft, WINDOWSIZE, STEPSIZE);
    readerbeat = new ReaderExtractBeat((ReaderExtract *)readerhfc, WINDOWSIZE, STEPSIZE, 100);
#endif
}

ReaderExtractWave::~ReaderExtractWave()
{
    delete [] temp;
    delete [] read_buffer;
    if (file)
        delete file;
#ifdef EXTRACT
    delete readerbeat;
    delete readerhfc;
    delete readerfft;
#endif
}

void ReaderExtractWave::newSource(TrackInfoObject *pTrack)
{
    //
    // Initialize new sound source
    //

    // If we are already playing a file, then get rid of the sound source:
    if (file != 0)
    {
        delete file;
        file = 0;
    }

    QString filename = pTrack->getLocation();
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
        }
    }
    else
    {
#ifdef __UNIX__
        file = new SoundSourceAudioFile( QString("/dev/null") );
#endif
#ifdef __WIN__
        file = new SoundSourceSndFile( QString("/dev/null") );
#endif
    }

    if (file==0)
        qFatal("Error opening %s", filename.latin1());

    // Initialize position in read buffer
    m_pReader->lock();
    filepos_start = 0;
    filepos_end = 0;
    filepos_play = 0;
    m_pReader->unlock();
    bufferpos_start = 0;
    bufferpos_end = 0;
    reset();

#ifdef EXTRACT
    readerfft->newSource(pTrack);
    readerhfc->newSource(pTrack);
    readerbeat->newSource(pTrack);
#endif
}

void ReaderExtractWave::addVisual(VisualChannel *pVisualChannel)
{
    ReaderExtract::addVisual(pVisualChannel);

#ifdef EXTRACT
//    readerfft->addVisual(pVisualChannel);
//    readerhfc->addVisual(pVisualChannel);
    readerbeat->addVisual(pVisualChannel);
#endif
}

void ReaderExtractWave::reset()
{
    m_pReader->lock();
    filepos_start = 0;
    filepos_end = 0;
    file->seek(0);
    m_pReader->unlock();

    bufferpos_start = 0;
    bufferpos_end = 0;

    for (unsigned int i=0; i<READBUFFERSIZE; i++)
        read_buffer[i] = 0.;

#ifdef EXTRACT
    // Reset extract objects
    readerfft->reset();
    readerhfc->reset();
    readerbeat->reset();
#endif

    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(0, READBUFFERSIZE));
}

void *ReaderExtractWave::getBasePtr()
{
    return (void *)read_buffer;
}

int ReaderExtractWave::getRate()
{
    if (file)
        return file->getSrate();
    else
        return 44100; // HACKKKK!!!!!
}

int ReaderExtractWave::getLength()
{
    if (file)
        return file->length();
    else
        return 0;
}

int ReaderExtractWave::getChannels()
{
    // Two channels waveform is hardcoded for the moment!!!
    return 2;
}

int ReaderExtractWave::getBufferSize()
{
    return READBUFFERSIZE;
}

ReaderExtractBeat *ReaderExtractWave::getExtractBeat()
{
    return readerbeat;
}
    
void *ReaderExtractWave::processChunk(const int, const int, const int, bool)
{
    return 0;
}

void ReaderExtractWave::getchunk(CSAMPLE rate)
{
    if (!file)
        return;

    // Determine playback direction
    bool backwards;
    if (rate < 0.)
    {
//        qDebug("rate back");
        backwards = true;
    }
    else
    {
//        qDebug("rate fwd");
        backwards = false;
    }

    //
    // Read a chunk. If playback direction is backwards we need to perform a seek, read forward, and perform a
    // seek to the end of the buffer. However, if we are reading samples before position 0 (this can happen to
    // reset the buffer) we have to avoid the two seeks because this can mess up the mp3 stream, because we
    // cannot perform an accurate seek.
    //

    // Determine which chunk to fetch, one back in time, or one ahead of time. This is to ensure that the
    // whole waveform display is updated, and ofcourse also that chunks in the playback direction is fetched
    // before playback.

/*
    if (!backwards && filepos_play>READCHUNKSIZE*(READCHUNK_NO)/2-1 && filepos_end>filepos_play && filepos_end-filepos_play>filepos_play-filepos_start)
        backwards = true;
    else if (backwards && filepos_play-filepos_start>filepos_end-filepos_play)
        backwards = false;
*/

    //qDebug(":::::::::: %i ::: %i",filepos_end-filepos_play,filepos_play-(filepos_start));
    if (!backwards && filepos_end>filepos_play && filepos_end-filepos_play>(filepos_play-filepos_start))
    {
//        qDebug("f-back");
        backwards = true;
    }
    else if (backwards && filepos_start<filepos_play && (filepos_play-filepos_start)>filepos_end-filepos_play)
    {
//        qDebug("f-fwd");
        backwards = false;
    }

//    qDebug("getchunk: pos %i, range %i-%i, back %i",filepos_play, filepos_start, filepos_end, backwards);

    //qDebug("play %i, range %i-%i",filepos_play,filepos_start,filepos_end);

    // Determine new start and end positions in file and buffer, start index of where read samples
    // will be placed in read buffer (bufIdx), and perform seek if reading backwards
    long int filepos_start_new, filepos_end_new;
    int bufIdx;

    m_pReader->lock();

    int chunkCurr, chunkStart, chunkEnd;

    bool seek = false;
    if (backwards)
    {
        filepos_start_new = filepos_start-READCHUNKSIZE;
        bufferpos_start = (bufferpos_start-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        //qDebug("filepos %f",filepos_start_new);
        {
//            qDebug("seek back");
            file->seek((long int)max(0,filepos_start_new));
            seek = true;
        }
        if ((filepos_end-filepos_start)/READCHUNKSIZE < (unsigned int)READCHUNK_NO)
            filepos_end_new = filepos_end;
        else
        {
            filepos_end_new = filepos_end-READCHUNKSIZE;
            bufferpos_end   = bufferpos_start; //(bufferpos_end-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        }

        bufIdx = bufferpos_start;

        chunkCurr = bufferpos_start/READCHUNKSIZE;
    }
    else
    {
        filepos_end_new = filepos_end+READCHUNKSIZE;
        bufIdx = bufferpos_end;
        bufferpos_end   = (bufferpos_end+READCHUNKSIZE)%READBUFFERSIZE;
        if ((filepos_end-filepos_start)/READCHUNKSIZE < (unsigned int)READCHUNK_NO)
            filepos_start_new = filepos_start;
        else
        {
            filepos_start_new = filepos_start+READCHUNKSIZE;
            bufferpos_start = bufferpos_end; //(bufferpos_start+READCHUNKSIZE)%READBUFFERSIZE;
        }
        chunkCurr = bufIdx/READCHUNKSIZE;
    }

    chunkStart = bufferpos_start/READCHUNKSIZE;
    chunkEnd   = bufferpos_end/READCHUNKSIZE;

    filepos_start = (long int)filepos_start_new;
    filepos_end = (long int)filepos_end_new;

    //qDebug("f %i-%i, b %i-%i",filepos_start, filepos_end, bufferpos_start, bufferpos_end);

    // Read samples (reset samples not read, but requested)
    int chunksize = READCHUNKSIZE;
    int k = 0;
    if (backwards && filepos_start<0)
    {
        //qDebug("d1 filepos %i-%i",filepos_start,filepos_end);
        int i;
        for (i=0; i<min((signed int)READCHUNKSIZE,-filepos_start); ++i)
            temp[i] = 0;
        //qDebug("i %i",i);
        chunksize += filepos_start;
        k = -filepos_start;
    }
    int i = 0;
    if (chunksize>0)
    {
        //qDebug("d2 filepos %i-%i",filepos_start,filepos_end);
        i = file->read(chunksize, &temp[k]);
        //qDebug("read %i",i);
        for (unsigned int j=i+k; j<READCHUNKSIZE; ++j)
            temp[j] = 0;
    }

    // Seek to end of the samples read in buffer, if we are reading backwards. This is to ensure, that the correct samples
    // are read, if we next time are going forward.
    if (seek)
    {
        //qDebug("seek fwd");
        file->seek((long int)filepos_end);
    }

    // Copy samples to read_buffer
    i=0;
    //qDebug("bufIdx %i",bufIdx);
    for (unsigned int j=bufIdx; j<bufIdx+READCHUNKSIZE; j++)
        read_buffer[j] = (CSAMPLE)temp[i++];

    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(bufIdx, READCHUNKSIZE));

#ifdef EXTRACT
    // Do pre-processing...
//    qDebug("curr %i, start %i, end %i",chunkCurr,chunkStart,chunkEnd);
    readerfft->processChunk(chunkCurr, chunkStart, chunkEnd, backwards);
    readerhfc->processChunk(chunkCurr, chunkStart, chunkEnd, backwards);
    readerbeat->processChunk(chunkCurr, chunkStart, chunkEnd, backwards);
#endif
    m_pReader->unlock();
}

long int ReaderExtractWave::seek(long int new_playpos)
{
    long int seekpos;

    if (file!=0)
    {
        m_pReader->lock();
        filepos_start = new_playpos;
        filepos_end = new_playpos;
        filepos_play = new_playpos;

        seekpos = file->seek((long int)filepos_start);

        //qDebug("seek: %i, %i",new_playpos, seekpos);

        m_pReader->unlock();

        bufferpos_start = 0;
        bufferpos_end = 0;

        unsigned int i;
        for (i=0; i<READBUFFERSIZE; i++)
            read_buffer[i] = 0.;

#ifdef EXTRACT
        // Reset extract objects
        readerfft->reset();
        readerhfc->reset();
        readerbeat->reset();
#endif

        // Update vertex buffer by sending an event containing indexes of where to update.
        if (m_pVisualBuffer != 0)
            QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(0,READBUFFERSIZE));
    }
    else
        seekpos = 0;

    return seekpos;
}
