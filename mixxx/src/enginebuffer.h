/***************************************************************************
                          enginebuffer.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEBUFFER_H
#define ENGINEBUFFER_H

#include <qapplication.h>
#include "defs.h"
#include "engineobject.h"
#include "monitor.h"

class ControlEngine;
class ControlObject;
class Reader;
class EngineBufferScale;
class PowerMate;
class WVisualWaveform;

/**
  *@author Tue and Ken Haste Andersen
*/

// Length of audio beat marks in samples
const int audioBeatMarkLen = 40;

// Rate at which the playpos slider is updated (using a sample rate of 44100 Hz):
const int UPDATE_RATE = 5;

// End of track mode constants
const int TRACK_END_MODE_STOP = 0;
const int TRACK_END_MODE_NEXT = 1;
const int TRACK_END_MODE_LOOP = 2;
const int TRACK_END_MODE_PING = 3;

class EngineBuffer : public EngineObject
{
    Q_OBJECT
public:
    EngineBuffer(PowerMate *, const char *_group);
    ~EngineBuffer();
    /** Pointer to visual widget */
    void setVisual(WVisualWaveform *pVisualWaveform);
    /** Returns pointer to Reader object. Used in MixxxApp. */
    Reader *getReader();
    /** Returns current bpm value (not thread-safe) */
    float getBpm();
    /** Return the current rate (not thread-safe) */
    float getRate();
    /** Return the distance to the next beat mark from curr. play pos (not thread-safe) */
    float getDistanceNextBeatMark();
    /** Reset buffer playpos and set file playpos. This must only be called while holding the
      * pause mutex */
    void setNewPlaypos(double);
    /** Sets pointer to other engine buffer/channel */
    void setOtherEngineBuffer(EngineBuffer *);
    
    CSAMPLE *process(const CSAMPLE *, const int);

    CSAMPLE *update_visual();
    const char *getGroup();
    /** Get the relative playpos in a buffer sampled at Srate hz*/
    int getPlaypos(int Srate);
    /** Notify used to call seek when playpos slider changes */
    Monitor visualPlaypos;
    float visualRate;
public slots:
    void slotControlPlay(double);
    void slotControlStart(double);
    void slotControlEnd(double);
    void slotControlSeek(double);
    void slotControlCueGoto(double=0);
    void slotControlCueSet(double=0);
    void slotControlCuePreview(double);
    void slotControlRatePermDown(double);
    void slotControlRatePermDownSmall(double);
    void slotControlRatePermUp(double);
    void slotControlRatePermUpSmall(double);
    void slotControlRateTempDown(double);
    void slotControlRateTempDownSmall(double);
    void slotControlRateTempUp(double);
    void slotControlRateTempUpSmall(double);
    void slotControlBeatSync(double);
//    void bpmChange(double);

private:
    /** Pointer to other EngineBuffer */
    EngineBuffer *m_pOtherEngineBuffer;
    /** Pointer to reader */
    Reader *reader;
    /** Buffer used in the process() */
    CSAMPLE *buffer;
    /** The current sample to play in the file. */
    double filepos_play;
    /** Sample in the buffer relative to filepos_play. */
    double bufferpos_play;
    /** Holds the rate as calculated in process. This is used in the
      * reader thread when calling soundbuffer->getchunk() */
    Monitor rate_exchange;
    /** Copy of rate_exchange, used to check if rate needs to be updated */
    double rate_old;
    /** Temp rate, used for pitch temp buttons */
    double temp_rate;
    /** Copy of length of file */
    int file_length_old;
    /** Copy of file sample rate*/
    int file_srate_old;
    /** Mutex controlling weather the process function is in pause mode. This happens
      * during seek and loading of a new track */
    QMutex pause;
    /** Used in update of playpos slider */
    int m_iSamplesCalculated;

    ControlEngine *playButton, *rateSlider, *wheel, *playposSlider, *bufferposSlider, *audioBeatMark;
    ControlEngine *buttonCueSet, *buttonCueGoto, *buttonCuePreview, *m_pRateDir;
    ControlEngine *buttonRateTempDown, *buttonRateTempDownSmall, *buttonRateTempUp, *buttonRateTempUpSmall;
    ControlEngine *buttonRatePermDown, *buttonRatePermDownSmall, *buttonRatePermUp, *buttonRatePermUpSmall;
    ControlEngine *buttonBeatSync;
    /** Control used to signal when at end of file */
    ControlEngine *m_pTrackEnd, *m_pTrackEndMode;
    /** Control used to input desired playback BPM */
    ControlEngine *bpmControl;
    /** Control used to input beat. If this is used, only one beat is played, until a new beat mark is received from the ControlObject */
    ControlEngine *beatEventControl;
    /** Reverse playback control */
    ControlEngine *reverseButton;
    /** Fwd and back controls, start and end of track control */
    ControlEngine *fwdButton, *backButton, *startButton, *endButton;
    /** Holds the name of the control group */
    const char *group;

    CSAMPLE *read_buffer_prt;

    /** Used to store if an event has happen in last iteration of event based playback */
    double oldEvent;
    /** Object used to perform waveform scaling (sample rate conversion) */
    EngineBufferScale *scale;
    /** Pointer to PowerMate object */
    PowerMate *powermate;
    /** Number of samples left in audio beat mark from last call to process */
    int m_iBeatMarkSamplesLeft;
};
#endif
