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
#ifdef __VISUALS__
#include "visual/visualchannel.h"
#include "visual/visualbuffer.h"
#endif

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
    readerbeat = new ReaderExtractBeat((ReaderExtract *)readerhfc, WINDOWSIZE, STEPSIZE, 200);
#endif

//    textout.setName("wave.txt");
//    textout.open( IO_WriteOnly );
}

ReaderExtractWave::~ReaderExtractWave()
{
    delete [] temp;
    delete [] read_buffer;
#ifdef EXTRACT
    delete readerbeat;
    delete readerhfc;
    delete readerfft;
#endif
}

void ReaderExtractWave::addVisual(VisualChannel *pVisualChannel)
{
    ReaderExtract::addVisual(pVisualChannel);

#ifdef EXTRACT
//    readerfft->addVisual(m_pVisualChannel);
//    readerhfc->addVisual(m_pVisualChannel);
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

#ifdef __VISUALS__
    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        for (int i=0; i<READBUFFERSIZE; i+=READCHUNKSIZE)
            QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(i, READCHUNKSIZE));
#endif
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

void ReaderExtractWave::setSoundSource(SoundSource *_file)
{
    file = _file;

    // Initialize position in read buffer
    m_pReader->lock();
    filepos_start = 0;
    filepos_end = 0;
    m_pReader->unlock();
    bufferpos_start = 0;
    bufferpos_end = 0;
    reset();
}

void ReaderExtractWave::getchunk(CSAMPLE rate)
{
//    QTextStream stream( &textout );


    //qDebug("Reading..., bufferpos_start %i",bufferpos_start);


    // Determine playback direction
    bool backwards;
    if (rate < 0.)
        backwards = true;
    else
        backwards = false;

    // Determine new start and end positions in file and buffer, start index of where read samples
    // will be placed in read buffer (bufIdx), and perform seek if reading backwards
    double filepos_start_new, filepos_end_new;
    int bufIdx;

    m_pReader->lock();

    int chunkCurr, chunkStart, chunkEnd;
    
    if (backwards)
    {        
        int preEnd = bufferpos_start;

        filepos_start_new = max(0.,filepos_start-(double)READCHUNKSIZE);
        bufferpos_start = (bufferpos_start-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        file->seek((long int)filepos_start_new);

        if ((filepos_end-filepos_start)/READCHUNKSIZE < READCHUNK_NO)
            filepos_end_new = filepos_end;
        else
        {
            filepos_end_new = filepos_end-(double)READCHUNKSIZE;
            bufferpos_end   = bufferpos_start; //(bufferpos_end-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        }
        
        bufIdx = bufferpos_start;

        chunkCurr = bufferpos_start/READCHUNKSIZE;
    }
    else
    {
        int preStart = bufferpos_end;

        filepos_end_new = filepos_end+(double)READCHUNKSIZE;
        bufIdx = bufferpos_end;
        bufferpos_end   = (bufferpos_end+READCHUNKSIZE)%READBUFFERSIZE;
        if ((filepos_end-filepos_start)/READCHUNKSIZE < READCHUNK_NO)
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
    
    filepos_start = filepos_start_new;
    filepos_end = filepos_end_new;

    // Read samples
    file->read(READCHUNKSIZE, temp);

    // Seek to end of the samples read in buffer, if we are reading backwards. This is to ensure, that the correct samples
    // are read, if we next time are going forward.
    if (backwards)
        file->seek((long int)filepos_end);

    // Copy samples to read_buffer
    int i=0;
    for (unsigned int j=bufIdx; j<bufIdx+READCHUNKSIZE; j++)
        read_buffer[j] = (CSAMPLE)temp[i++];

    // Write wave to text file
//    for (int j=bufIdx; j<bufIdx+READCHUNKSIZE; j+=2)
//        stream << read_buffer[j] << "\n";
//    //stream << "\n";
//    textout.flush();

        
#ifdef EXTRACT
    // Do pre-processing...
//    qDebug("curr %i, start %i, end %i",chunkCurr,chunkStart,chunkEnd);
    readerfft->processChunk(chunkCurr, chunkStart, chunkEnd, backwards);
    readerhfc->processChunk(chunkCurr, chunkStart, chunkEnd, backwards);
    readerbeat->processChunk(chunkCurr, chunkStart, chunkEnd, backwards);
#endif
    m_pReader->unlock();

#ifdef __VISUALS__
    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(bufIdx, READCHUNKSIZE));
#endif
}

long int ReaderExtractWave::seek(long int new_playpos)
{
    m_pReader->lock();
    filepos_start = new_playpos;
    filepos_end = new_playpos;

    long int seekpos = file->seek((long int)filepos_start);

    qDebug("seek: %i, %i",new_playpos, seekpos);
    
    m_pReader->unlock();

    bufferpos_start = 0;
    bufferpos_end = 0;

    for (unsigned int i=0; i<READBUFFERSIZE; i++)
        read_buffer[i] = 0.;

#ifdef EXTRACT
    // Reset extract objects
    readerfft->reset();
    readerhfc->reset();
    readerbeat->softreset(); // Only make a soft reset on beat estimation (keep histogram)
#endif

#ifdef __VISUALS__
    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        for (int i=0; i<READBUFFERSIZE; i+=READCHUNKSIZE)
            QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(i,READCHUNKSIZE));
#endif

    return seekpos;
}
