/***************************************************************************
                          soundbuffer.cpp  -  description
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

#include "soundbuffer.h"

SoundBuffer::SoundBuffer(int _chunkSize, int _chunkNo, int windowSize, int _stepSize, GUIChannel *guichannel)
{
    // Setup variables for block based analysis
    chunkSize = _chunkSize;
    chunkNo = _chunkNo;
    stepSize = _stepSize;
    windowPerChunk = chunkSize/stepSize;
    windowNo = chunkNo*windowPerChunk;    
    
    // Allocate and calculate window
    window = new WindowKaiser(windowSize, 6.5);

    // Allocate memory for windowed portion of signal
    windowedSamples = new CSAMPLE[windowSize];

    // Allocate temporary buffer
    temp = new SAMPLE[READCHUNKSIZE*2]; // Temporary buffer for the raw samples

    // Allocate read_buffer
    read_buffer = new CSAMPLE[READBUFFERSIZE];
    for (unsigned i=0; i<READBUFFERSIZE; ++i)
        read_buffer[i] = 0.;

    // Allocate pre processing object
    preprocess = new EnginePreProcess(this, windowNo, window);
        
    visualBuffer = 0;
        
    // Initialize position in read buffer
    filepos_start.write(0.);
    filepos_end.write(0.);
    bufferpos_start = 0;
    bufferpos_end = 0;
}

SoundBuffer::~SoundBuffer()
{
    delete [] temp;
    delete [] read_buffer;
    delete [] window;
    delete preprocess;
}

void SoundBuffer::setSoundSource(SoundSource *_file)
{
    file = _file;
}

/*
  Read a new chunk into the readbuffer:
*/
void SoundBuffer::getchunk(CSAMPLE rate)
{
    //qDebug("Reading..., playpos %f",playpos_buffer.read());


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
    if (backwards)
    {        
        int preEnd = bufferpos_start;

        filepos_start_new = max(0.,filepos_start.read()-(double)READCHUNKSIZE);
        bufferpos_start = (bufferpos_start-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        file->seek((long int)filepos_start_new);

        if ((filepos_end.read()-filepos_start_new)/READCHUNKSIZE < READCHUNK_NO)
            filepos_end_new = filepos_end.read();
        else
        {
            filepos_end_new = filepos_end.read()-(double)READCHUNKSIZE;
            bufferpos_end   = bufferpos_start; //(bufferpos_end-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        }

        bufIdx = bufferpos_start;

        // Do pre-processing.
        preprocess->update((bufferpos_start/stepSize+1)%windowNo, (preEnd/stepSize-1+windowNo)%windowNo);
    }
    else
    {
        int preStart = bufferpos_end;

        filepos_end_new = filepos_end.read()+(double)READCHUNKSIZE;
        bufIdx = bufferpos_end;
        bufferpos_end   = (bufferpos_end+READCHUNKSIZE)%READBUFFERSIZE;
        if ((filepos_end.read()-filepos_start.read())/READCHUNKSIZE < READCHUNK_NO)
            filepos_start_new = filepos_start.read();
        else
        {
            filepos_start_new = filepos_start.read()+READCHUNKSIZE;
            bufferpos_start = bufferpos_end; //(bufferpos_start+READCHUNKSIZE)%READBUFFERSIZE;
        }

        // Do pre-processing...
        preprocess->update((preStart/stepSize+1)%windowNo, (bufferpos_end/stepSize-1+windowNo)%windowNo);
    }
    filepos_start.write(filepos_start_new);
    filepos_end.write(filepos_end_new);

    // Read samples
    file->read(READCHUNKSIZE, temp);

    // Seek to end of the samples read in buffer, if we are reading backwards. This is to ensure, that the correct samples
    // are read, if we next time are going forward.
    if (backwards)
        file->seek((long int)filepos_end.read());

    // Copy samples to read_buffer
    int i=0;
    for (unsigned int j=bufIdx; j<bufIdx+READCHUNKSIZE; j++)
        read_buffer[j] = (CSAMPLE)temp[i++];

    // Update variables used to copy the buffer to a vertex buffer for 3D visualization
    visualPos1 = bufIdx;
    visualLen1 = READCHUNKSIZE;
    visualPos2 = 0;
    visualLen2 = 0;

    // Send user event to main thread, indicating that the visual sample buffer should be updated
    if (visualBuffer>0)
        QThread::postEvent(visualBuffer,new QEvent((QEvent::Type)1001));
}

/*
  Reset data in read_buffer
*/
void SoundBuffer::reset(double new_playpos)
{
    filepos_start.write(new_playpos);
    filepos_end.write(new_playpos);

    bufferpos_start = 0;
    bufferpos_end = 0;

    for (unsigned int i=0; i<READBUFFERSIZE; i++)
        read_buffer[i] = 0.;
}

// Get a pointer to the chunk at index chunkIdx
CSAMPLE *SoundBuffer::getChunkPtr(int chunkIdx)
{
    return &read_buffer[chunkSize*chunkIdx];
}

// Get a pointer to a window centered around the sample at windowIdx*windowSize
CSAMPLE *SoundBuffer::getWindowPtr(int windowIdx)
{
    // Start position of window
    int windowPos = (windowIdx*stepSize-window->getSize()/2+READBUFFERSIZE)%READBUFFERSIZE;

//    qDebug("windowIdx %i, windowPos %i, bufferpos_end %i, bufferpos_start %i",windowIdx,windowPos,bufferpos_end, bufferpos_start);
/*    if (windowPos+window->getSize()>=bufferpos_end || windowPos<bufferpos_start)
    {
        return 0;
    }
    else
*/
    {
        if (windowPos+window->getSize() < chunkNo*chunkSize)
            for (int i=windowPos; i<windowPos+window->getSize(); i++)
                windowedSamples[i-windowPos] = read_buffer[i];
        else
        {
            for (int i=windowPos; i<chunkNo*chunkSize; i++)
                windowedSamples[i-windowPos] = read_buffer[i];
            for (int i=0; i<(windowPos+window->getSize())%READBUFFERSIZE; i++)
                windowedSamples[i-windowPos] = read_buffer[i];
        }
    }
    return windowedSamples;    
}

double SoundBuffer::getFileposStart()
{
    return filepos_start.read();
}

double SoundBuffer::getFileposEnd()
{
    return filepos_end.read();
}

void SoundBuffer::setVisual(QObject *_visualBuffer)
{
    visualBuffer = _visualBuffer;
}