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
#include <qmutex.h>
#include "defs.h"
#include "engineobject.h"
#include "configobject.h"
#include "monitor.h"
#include "rotary.h"

class RateControl;
class LoopingControl;
class ControlObject;
class ControlPushButton;
class ControlBeat;
class ControlTTRotary;
class ControlPotmeter;
class Reader;
class EngineBufferScale;
class EngineBufferScaleLinear;
class EngineBufferScaleST;
class EngineBufferCue;

/**
  *@author Tue and Ken Haste Andersen
*/

// Length of audio beat marks in samples
const int audioBeatMarkLen = 40;

// Temporary buffer length
const int kiTempLength = 200000;

// Rate at which the playpos slider is updated (using a sample rate of 44100 Hz):
const int UPDATE_RATE = 5;

// End of track mode constants
const int TRACK_END_MODE_STOP = 0;
const int TRACK_END_MODE_NEXT = 1;
const int TRACK_END_MODE_LOOP = 2;
const int TRACK_END_MODE_PING = 3;

// Maximum number of samples used to ramp to or from zero when playback is
// stopped or started.
const int kiRampLength = 50;


class EngineBuffer : public EngineObject
{
    Q_OBJECT
public:
    EngineBuffer(const char *_group, ConfigObject<ConfigValue> *_config);
    ~EngineBuffer();
    /** Reconfigures the EngineBufferScaleSRC objects with the sound scale mode written in the config database */
    void setPitchIndpTimeStretch(bool b);
    bool getPitchIndpTimeStretch(void);

    /** Returns pointer to Reader object. Used in MixxxApp. */
    Reader *getReader();
    /** Return the current rate (not thread-safe) */
    double getRate();
    /** Returns current bpm value (not thread-safe) */
    double getBpm();
    /** Sets pointer to other engine buffer/channel */
    void setOtherEngineBuffer(EngineBuffer *);
    /** Reset buffer playpos and set file playpos. This must only be called while holding the
      * pause mutex */
    void setNewPlaypos(double);

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void processOld(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

    const char *getGroup();

    /** Lock abs and buffer playpos vars, so that they can be accessed through
      * getBufferPlaypos() and getAbsPlaypos() from another thread */
    void lockPlayposVars();
    /** Unlocks abs and buffer playpos vars, so that they can be accessed
      * internally in this object */
    void unlockPlayposVars();
    /** Returns the buffer playpos. lockPlayposVars() must be called in advance */
    double getBufferPlaypos();
    /** Returns the abs playpos. lockPlayposVars() must be called in advance */
    double getAbsPlaypos();
    /** Returns the buffer startpos. lockPlayposVars() must be called in advance */    
    double getAbsStartpos();


public slots:
    void slotControlPlay(double);
    void slotControlStart(double);
    void slotControlEnd(double);
    void slotControlSeek(double, bool bBeatSync=true);
    void slotControlSeekAbs(double, bool bBeatSync=true);
    void slotControlBeatSync(double);
    void slotSetBpm(double);
    void slotControlFastFwd(double);
    void slotControlFastBack(double);

private:

    /** Called from process() when an empty buffer, possible ramped to zero is needed */
    void rampOut(const CSAMPLE *pOut, int iBufferSize);

    /** Pointer to the loop control object */
    LoopingControl *m_pLoopingControl;
    
    /** Pointer to the rate control object */
    RateControl *m_pRateControl;

    /** Pointer to other EngineBuffer */
    EngineBuffer *m_pOtherEngineBuffer;
    /** Pointer to reader */
    Reader *reader;
    /** The current sample to play in the file. */
    double filepos_play;
    /** Sample in the buffer relative to filepos_play. */
    double bufferpos_play;
    /** Copy of rate_exchange, used to check if rate needs to be updated */
    double rate_old;
    /** Copy of length of file */
    long int file_length_old;
    /** Copy of file sample rate*/
    int file_srate_old;
    /** Mutex controlling weather the process function is in pause mode. This happens
      * during seek and loading of a new track */
    QMutex pause;
    /** Used in update of playpos slider */
    int m_iSamplesCalculated;

    ControlPushButton *playButton, *buttonBeatSync;

    ControlObject *rateEngine, *m_pRateDir, *m_pRateRange;
    ControlObject *m_pMasterRate;
	ControlObject *m_pJog;
    ControlPotmeter *rateSlider, *m_pRateSearch;
    ControlTTRotary *wheel, *m_pControlScratch;
    ControlPushButton *wheelTouchSensor, *wheelTouchSwitch;
    ControlPotmeter *playposSlider;
    ControlPotmeter *visualPlaypos;
    ControlObject *m_pFileBpm, *m_pSampleRate;
    ConfigObject<ConfigValue> *m_pConfig; 
                 
    /** Mutex used in sharing buffer and abs playpos */
    QMutex m_qPlayposMutex;
    /** Buffer and absolute playpos shared among threads */
    double m_dBufferPlaypos, m_dAbsPlaypos, m_dAbsStartpos;

    /** Control used to signal when at end of file */
    ControlObject *m_pTrackEnd, *m_pTrackEndMode;
    /** Control used to input desired playback BPM */
    ControlBeat *bpmControl;
    /** Reverse playback control */
    ControlPushButton *reverseButton;
    /** Fwd and back controls, start and end of track control */
    ControlPushButton *fwdButton, *backButton, *startButton, *endButton;
    /** Holds the name of the control group */
    const char *group;

    CSAMPLE *read_buffer_prt;

    /** Pointer to cue object */
    EngineBufferCue *m_pEngineBufferCue;
    
    /** Used to store if an event has happen in last iteration of event based playback */
    double oldEvent;
    /** Object used to perform waveform scaling (sample rate conversion) */
    EngineBufferScale *m_pScale;
    /** Object used for linear interpolation scaling of the audio */
    EngineBufferScaleLinear *m_pScaleLinear;
    /** Object used for pitch-indep time stretch (key lock) scaling of the audio */
    EngineBufferScaleST *m_pScaleST;
    /** Number of samples left in audio beat mark from last call to process */
    int m_iBeatMarkSamplesLeft;
    /** Holds the last sample value of the previous buffer. This is used when ramping to
      * zero in case of an immediate stop of the playback */
    float m_fLastSampleValue;
    /** Is true if the previous buffer was silent due to pausing */
    bool m_bLastBufferPaused;
    /** Temporary buffer used when seeking and looping */
    float *m_pTempBuffer;
    /** Temproray buffer position in file */
    double m_dTempFilePos; // TODO

    /** Pointer to ReaderExtractWave buffer */
    float *m_pWaveBuffer;

    /** Whether Pitch-Independent Time Stretch should be re-enabled when we start playing post-scratch **/
    bool m_bResetPitchIndpTimeStretch; // TODO

    // Filter jog wheel data to smooth it:
    Rotary* m_jogfilter;
};
#endif
