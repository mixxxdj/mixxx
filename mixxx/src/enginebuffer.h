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
#include "monitor.h"

class ControlObject;
class ControlPushButton;
class ControlBeat;
class ControlTTRotary;
class ControlPotmeter;
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

// Maximum number of samples used to ramp to or from zero when playback is
// stopped or started.
const int kiRampLength = 50;


class EngineBuffer : public EngineObject
{
    Q_OBJECT
public:
    EngineBuffer(PowerMate *, const char *_group);
    ~EngineBuffer();
    /** Reconfigures the EngineBufferScaleSRC objects with the sound quality written in the config database */
    void setQuality(int);
    /** Pointer to visual widget */
    void setVisual(WVisualWaveform *pVisualWaveform);
    /** Returns pointer to Reader object. Used in MixxxApp. */
    Reader *getReader();
    /** Returns current bpm value (not thread-safe) */
    double getBpm();
    /** Return the current rate (not thread-safe) */
    double getRate();
    /** Return the distance to the next beat mark from curr. play pos (not thread-safe) */
    float getDistanceNextBeatMark();
    /** Reset buffer playpos and set file playpos. This must only be called while holding the
      * pause mutex */
    void setNewPlaypos(double);
    /** Sets pointer to other engine buffer/channel */
    void setOtherEngineBuffer(EngineBuffer *);

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

    CSAMPLE *update_visual();
    const char *getGroup();
    /** Set rate change when temp rate button is pressed */
    static void setTemp(double v);
    /** Set rate change when temp rate small button is pressed */
    static void setTempSmall(double v);
    /** Set rate change when perm rate button is pressed */
    static void setPerm(double v);
    /** Set rate change when perm rate small button is pressed */
    static void setPermSmall(double v);
    /** Notify used to call seek when playpos slider changes */
    Monitor visualPlaypos;
    float visualRate;

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


public slots:
    void slotControlPlay(double);
    void slotControlStart(double);
    void slotControlEnd(double);
    void slotControlSeek(double, bool bBeatSync=true);
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
    void slotSetBpm(double);
    void slotControlFastFwdBack(double);

private:
    /** Called from process() when an empty buffer, possible ramped to zero is needed */
    void rampOut(const CSAMPLE *pOut, int iBufferSize);
    /** Adjust beat phase */
    void adjustPhase();

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
    int file_length_old;
    /** Copy of file sample rate*/
    int file_srate_old;
    /** Mutex controlling weather the process function is in pause mode. This happens
      * during seek and loading of a new track */
    QMutex pause;
    /** Used in update of playpos slider */
    int m_iSamplesCalculated;
    /** Values used when temp and perm rate buttons are pressed */
    static double m_dTemp, m_dTempSmall, m_dPerm, m_dPermSmall;
    /** Is true if a rate temp button is pressed */
    double m_bTempPress;

    ControlPushButton *playButton, *buttonCueSet, *buttonCueGoto, *buttonCuePreview, *audioBeatMark, *buttonBeatSync;
    ControlPushButton *buttonRateTempDown, *buttonRateTempDownSmall, *buttonRateTempUp, *buttonRateTempUpSmall;
    ControlPushButton *buttonRatePermDown, *buttonRatePermDownSmall, *buttonRatePermUp, *buttonRatePermUpSmall;
    ControlObject *cuePoint, *rateEngine, *m_pRateDir, *m_pRateRange;
    ControlPotmeter *rateSlider, *playposSlider;
    ControlTTRotary *wheel, *m_pControlScratch;

    /** Mutex used in sharing buffer and abs playpos */
    QMutex m_qPlayposMutex;
    /** Buffer and absolute playpos shared among threads */
    double m_dBufferPlaypos, m_dAbsPlaypos;

    /** Control used to signal when at end of file */
    ControlObject *m_pTrackEnd, *m_pTrackEndMode;
    /** Control used to input desired playback BPM */
    ControlBeat *bpmControl;
    /** Control used to input beat. If this is used, only one beat is played, until a new beat mark is received from the ControlObject */
    ControlPotmeter *beatEventControl;
    /** Reverse playback control */
    ControlPushButton *reverseButton;
    /** Fwd and back controls, start and end of track control */
    ControlPushButton *fwdButton, *backButton, *startButton, *endButton;
    /** Holds the name of the control group */
    const char *group;
    /** Is true if currently in cue preview mode. We need to keep track of the state
      * because the preview key slot can be activated many times during one preview */
    bool m_bCuePreview;

    CSAMPLE *read_buffer_prt;

    /** Used to store if an event has happen in last iteration of event based playback */
    double oldEvent;
    /** Object used to perform waveform scaling (sample rate conversion) */
    EngineBufferScale *scale;
    /** Pointer to PowerMate object */
    PowerMate *powermate;
    /** Number of samples left in audio beat mark from last call to process */
    int m_iBeatMarkSamplesLeft;
    /** Holds the last sample value of the previous buffer. This is used when ramping to
      * zero in case of an immediate stop of the playback */
    float m_fLastSampleValue;
    /** Is true if the previous buffer was silent due to pausing */
    bool m_bLastBufferPaused;
    /** Temporary seek buffer */
    float *m_pTempSeekBuffer;
    /** Temproray seek buffer position in file */
    double m_dTempSeekFilePos;
    /** Seek position */
    double m_dSeekFilePos;
    /** True if currently performing a beat syncronious seek */
    bool m_bSeekBeat;
    /** True if crossfade has been performed (is only used when m_bSeekBeat is true) */
    bool m_bSeekCrossfade;
    /** File position of cross fade end */
    double m_dSeekCrossfadeEndFilePos;
    /** Pointer to ReaderExtractWave buffer */
    float *m_pWaveBuffer;
    /** Variables used to calculate safe beat info. Automatically updated during process() */
    double m_dBeatFirst, m_dBeatInterval;

};
#endif
