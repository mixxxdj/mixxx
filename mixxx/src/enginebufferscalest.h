/***************************************************************************
                          enginebufferscalest.h  -  description
                             -------------------
	begin                : November 2004
	copyright            : (C) 2004 by Tue & Ken Haste Andersen
	email                : haste@diku.dk
	adaptation           : Mads Holm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEBUFFERSCALEST_H
#define ENGINEBUFFERSCALEST_H

#include "enginebufferscale.h"
#include "SoundTouch.h"

#define MAXBUFFERBLOCKS 100
#define ReadAheadSize 1000

using namespace soundtouch;

/**
  * Performs time scaling of audio based on the SoundTouch library.
  * Adapted by Mads Holm, based on work by Tue & Ken Haste Andersen
  * The SoundTouch Library is courtesy of Olli Parviainen, released
  * under the GNU Lesser General Public License.
  */

// Info structures for keeping track of buffered sound data
class ScaleBufferBlock 
{
public:
    double ScaledSamples;
    double UsedSamples;
};

class ScaleBufferDescriptor 
{
private:
    ScaleBufferBlock *pScaleBlocks;
    int Head;
    int Tail;
public:
    ScaleBufferDescriptor();
    ~ScaleBufferDescriptor();
    void append(double ScaledSamples, double UsedSamples);
    void pop();
    bool empty();
    ScaleBufferBlock *getHead();
    void showDebug();
    void clear();
};

class EngineBufferScaleST : public EngineBufferScale  
{
public: 
    EngineBufferScaleST(ReaderExtractWave *wave);
    ~EngineBufferScaleST();
    
    void setPitchIndpTimeStretch(bool b);

    CSAMPLE *scale(double playpos, int buf_size, float *pBase=0, int iBaseLength=0);

    /* This one is actually called from the engine
     * Should be replaced by a call to setTempo.  */
    double setRate(double rate);
    
    /* This one should be called instead of setRate */
    double setTempo(double _tempo);

    //flush buffer
    void clear();

private:
    // reset does the real work of clear() 
    void reset(double _playposition,bool _backwards);
    bool _clear_pending;

    // Info structure for keeping track of buffered sound data
    ScaleBufferDescriptor *pBlocks;

    /** Holds the playback direction */
    bool backwards;
    bool old_backwards;

    /** Buffer used to reverse output from SoundTouch 
      * library when playback direction is backwards */
    CSAMPLE *buffer_back;

    /* SoundTouch time/pitch scaling lib */
    SoundTouch *pSoundTouch;

    /* Sample rate of soundfile last time checked */
    uint oldSampleRate;

    /* Index of the actual read position */
    double readAheadPos;

    /* Actual rate used when calling SoundTouch */
    double rateUsed;

    /* Actual tempo used when calling SoundTouch */
    double tempoUsed;

    /* Expected input parameter playpos when scale is called next time */
    /* Used for detecting seeks, so clear() can be issued properly */
    double ajourplaypos;

    /* EngineBuffer::getPlaySrate() returns 0 so this one replaces it for now */
    uint getPlaySrate(){return 44100;};

    bool m_bPitchIndpTimeStretch;
};

#endif
