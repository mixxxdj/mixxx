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

#include <QMutex>
#include <QAtomicInt>
#include <gtest/gtest_prod.h>

#include "engine/cachingreader/cachingreader.h"
#include "preferences/usersettings.h"
#include "control/controlvalue.h"
#include "engine/engineobject.h"
#include "engine/sync/syncable.h"
#include "track/track.h"
#include "util/rotary.h"
#include "util/types.h"

//for the writer
#ifdef __SCALER_DEBUG__
#include <QFile>
#include <QTextStream>
#endif

class EngineChannel;
class EngineControl;
class BpmControl;
class KeyControl;
class RateControl;
class SyncControl;
class VinylControlControl;
class LoopingControl;
class ClockControl;
class CueControl;
class ReadAheadManager;
class ControlObject;
class ControlProxy;
class ControlPushButton;
class ControlIndicator;
class ControlBeat;
class ControlTTRotary;
class ControlPotmeter;
class EngineBufferScale;
class EngineBufferScaleLinear;
class EngineBufferScaleST;
class EngineBufferScaleRubberBand;
class EngineSync;
class EngineWorkerScheduler;
class VisualPlayPosition;
class EngineMaster;

/**
  *@author Tue and Ken Haste Andersen
*/

// Length of audio beat marks in samples
const int audioBeatMarkLen = 40;

// Temporary buffer length
const int kiTempLength = 200000;

// Rate at which the playpos slider is updated
const int kiPlaypositionUpdateRate = 15; // updates per second

class EngineBuffer : public EngineObject {
     Q_OBJECT
  private:
    enum SyncRequestQueued {
        SYNC_REQUEST_NONE,
        SYNC_REQUEST_ENABLE,
        SYNC_REQUEST_DISABLE,
        SYNC_REQUEST_ENABLEDISABLE,
    };
  public:
    enum SeekRequest {
        SEEK_NONE = 0x00,
        SEEK_PHASE = 0x01, // This is set to force an in-phase seek.
        SEEK_EXACT = 0x02, // This is used to seek to position regardless of
                           // if Quantize is enabled.
        SEEK_EXACT_PHASE = SEEK_PHASE | SEEK_EXACT,
						   // This is an artificial state that happens if
                           // an exact seek and a phase seek are scheduled
                           // at the same time.
        SEEK_STANDARD = 0x04, // This seeks to the exact position if Quantize is
                              // disabled or performs an in-phase seek if it is enabled.
        SEEK_STANDARD_PHASE = SEEK_STANDARD | SEEK_PHASE,
                              // This is an artificial state that happens if
                              // a standard seek and a phase seek are scheduled
                              // at the same time.
    };
    Q_DECLARE_FLAGS(SeekRequests, SeekRequest);

    enum KeylockEngine {
        SOUNDTOUCH,
        RUBBERBAND,
        KEYLOCK_ENGINE_COUNT,
    };

    EngineBuffer(const QString& group, UserSettingsPointer pConfig,
                 EngineChannel* pChannel, EngineMaster* pMixingEngine);
    virtual ~EngineBuffer();

    void bindWorkers(EngineWorkerScheduler* pWorkerScheduler);

    // Return the current rate (not thread-safe)
    double getSpeed();
    bool getScratching();
    // Returns current bpm value (not thread-safe)
    double getBpm();
    // Returns the BPM of the loaded track around the current position (not thread-safe)
    double getLocalBpm();
    // Sets pointer to other engine buffer/channel
    void setEngineMaster(EngineMaster*);

    // Queues a new seek position. Use SEEK_EXACT or SEEK_STANDARD as seekType
    void queueNewPlaypos(double newpos, enum SeekRequest seekType);
    void requestSyncPhase();
    void requestEnableSync(bool enabled);
    void requestSyncMode(SyncMode mode);
    void requestClonePosition(EngineChannel* pChannel);

    // The process methods all run in the audio callback.
    void process(CSAMPLE* pOut, const int iBufferSize);
    void processSlip(int iBufferSize);
    void postProcess(const int iBufferSize);

    QString getGroup();
    bool isTrackLoaded();
    // return true if a seek is currently cueued but not yet processed, false otherwise
    // if no seek was queued, the seek position is set to -1
    bool getQueuedSeekPosition(double* pSeekPosition);
    TrackPointer getLoadedTrack() const;

    bool isReverse();

    double getExactPlayPos();
    double getVisualPlayPos();
    double getTrackSamples();

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

    // For dependency injection of readers.
    //void setReader(CachingReader* pReader);

    // For dependency injection of scalers.
    void setScalerForTest(EngineBufferScale* pScaleVinyl,
                          EngineBufferScale* pScaleKeylock);

    // For injection of fake tracks.
    void loadFakeTrack(TrackPointer pTrack, bool bPlay);

    static QString getKeylockEngineName(KeylockEngine engine) {
        switch (engine) {
        case SOUNDTOUCH:
            return tr("Soundtouch (faster)");
        case RUBBERBAND:
            return tr("Rubberband (better)");
        default:
            return tr("Unknown (bad value)");
        }
    }

    // Request that the EngineBuffer load a track. Since the process is
    // asynchronous, EngineBuffer will emit a trackLoaded signal when the load
    // has completed.
    void loadTrack(TrackPointer pTrack, bool play);

  public slots:
    void slotControlPlayRequest(double);
    void slotControlPlayFromStart(double);
    void slotControlJumpToStartAndStop(double);
    void slotControlStop(double);
    void slotControlStart(double);
    void slotControlEnd(double);
    void slotControlSeek(double);
    void slotControlSeekAbs(double);
    void slotControlSeekExact(double);
    void slotKeylockEngineChanged(double);

    void slotEjectTrack(double);

  signals:
    void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void trackLoadFailed(TrackPointer pTrack, QString reason);

  private slots:
    void slotTrackLoading();
    void slotTrackLoaded(TrackPointer pTrack,
                         int iSampleRate, int iNumSamples);
    void slotTrackLoadFailed(TrackPointer pTrack,
                             QString reason);
    // Fired when passthrough mode is enabled or disabled.
    void slotPassthroughChanged(double v);

  private:
    // Add an engine control to the EngineBuffer
    // must not be called outside the Constructor
    void addControl(EngineControl* pControl);

    void enableIndependentPitchTempoScaling(bool bEnable,
                                            const int iBufferSize);

    void updateIndicators(double rate, int iBufferSize);

    void hintReader(const double rate);

    void ejectTrack();

    double fractionalPlayposFromAbsolute(double absolutePlaypos);

    void doSeekFractional(double fractionalPos, enum SeekRequest seekType);
    void doSeekPlayPos(double playpos, enum SeekRequest seekType);

    // Read one buffer from the current scaler into the crossfade buffer.  Used
    // for transitioning from one scaler to another, or reseeking a scaler
    // to prevent pops.
    void readToCrossfadeBuffer(const int iBufferSize);

    // Copy the play position from the given buffer
    void seekCloneBuffer(EngineBuffer* pOtherBuffer);

    // Reset buffer playpos and set file playpos.
    void setNewPlaypos(double playpos);

    void processSyncRequests();
    void processSeek(bool paused);

    bool updateIndicatorsAndModifyPlay(bool newPlay);
    void verifyPlay();
    void notifyTrackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void processTrackLocked(CSAMPLE* pOutput, const int iBufferSize, int sample_rate);

    // Holds the name of the control group
    QString m_group;
    UserSettingsPointer m_pConfig;

    friend class CueControlTest;

    LoopingControl* m_pLoopingControl; // used for testes
    FRIEND_TEST(LoopingControlTest, LoopScale_HalvesLoop);
    FRIEND_TEST(LoopingControlTest, LoopMoveTest);
    FRIEND_TEST(LoopingControlTest, LoopResizeSeek);
    FRIEND_TEST(LoopingControlTest, ReloopToggleButton_DoesNotJumpAhead);
    FRIEND_TEST(LoopingControlTest, ReloopAndStopButton);
    FRIEND_TEST(LoopingControlTest, Beatjump_JumpsByBeats);
    FRIEND_TEST(SyncControlTest, TestDetermineBpmMultiplier);
    FRIEND_TEST(EngineSyncTest, HalfDoubleBpmTest);
    FRIEND_TEST(EngineSyncTest, HalfDoubleThenPlay);
    FRIEND_TEST(EngineSyncTest, UserTweakBeatDistance);
    FRIEND_TEST(EngineBufferTest, ScalerNoTransport);
    EngineSync* m_pEngineSync;
    SyncControl* m_pSyncControl;
    VinylControlControl* m_pVinylControlControl;
    RateControl* m_pRateControl;
    BpmControl* m_pBpmControl;
    KeyControl* m_pKeyControl;
    ClockControl* m_pClockControl;
    CueControl* m_pCueControl;

    QList<EngineControl*> m_engineControls;

    // The read ahead manager for EngineBufferScale's that need to read ahead
    ReadAheadManager* m_pReadAheadManager;

    // The reader used to read audio files
    CachingReader* m_pReader;

    // List of hints to provide to the CachingReader
    HintVector m_hintList;

    // The current sample to play in the file.
    double m_filepos_play;

    // The previous callback's speed. Used to check if the scaler parameters
    // need updating.
    double m_speed_old;

    // The previous callback's tempo ratio.
    double m_tempo_ratio_old;

    // True if the previous callback was scratching.
    bool m_scratching_old;

    // True if the previous callback was reverse.
    bool m_reverse_old;

    // The previous callback's pitch. Used to check if the scaler parameters
    // need updating.
    double m_pitch_old;

    // The previous callback's baserate. Used to check if the scaler parameters
    // need updating.
    double m_baserate_old;

    // Copy of rate_exchange, used to check if rate needs to be updated
    double m_rate_old;

    // Copy of length of file
    int m_trackSamplesOld;

    // Copy of file sample rate
    double m_trackSampleRateOld;

    // Mutex controlling whether the process function is in pause mode. This happens
    // during seek and loading of a new track
    QMutex m_pause;
    // Used in update of playpos slider
    int m_iSamplesSinceLastIndicatorUpdate;

    // The location where the track would have been had slip not been engaged
    double m_dSlipPosition;
    // Saved value of rate for slip mode
    double m_dSlipRate;
    // m_bSlipEnabledProcessing is only used by the engine processing thread.
    bool m_bSlipEnabledProcessing;

    ControlObject* m_pTrackSamples;
    ControlObject* m_pTrackSampleRate;

    ControlPushButton* m_playButton;
    ControlPushButton* m_playStartButton;
    ControlPushButton* m_stopStartButton;
    ControlPushButton* m_stopButton;

    ControlObject* m_fwdButton;
    ControlObject* m_backButton;
    ControlPushButton* m_pSlipButton;

    ControlObject* m_pQuantize;
    ControlObject* m_pMasterRate;
    ControlPotmeter* m_playposSlider;
    ControlProxy* m_pSampleRate;
    ControlProxy* m_pKeylockEngine;
    ControlPushButton* m_pKeylock;

    // This ControlProxys is created as parent to this and deleted by
    // the Qt object tree. This helps that they are deleted by the creating
    // thread, which is required to avoid segfaults.
    ControlProxy* m_pPassthroughEnabled;

    ControlPushButton* m_pEject;
    ControlObject* m_pTrackLoaded;

    // Whether or not to repeat the track when at the end
    ControlPushButton* m_pRepeat;

    // Fwd and back controls, start and end of track control
    ControlPushButton* m_startButton;
    ControlPushButton* m_endButton;

    // Object used to perform waveform scaling (sample rate conversion).  These
    // three pointers may be reassigned depending on configuration and tests.
    EngineBufferScale* m_pScale;
    FRIEND_TEST(EngineBufferTest, SlowRubberBand);
    FRIEND_TEST(EngineBufferTest, ResetPitchAdjustUsesLinear);
    FRIEND_TEST(EngineBufferTest, VinylScalerRampZero);
    FRIEND_TEST(EngineBufferTest, ReadFadeOut);
    FRIEND_TEST(EngineBufferTest, RateTempTest);
    FRIEND_TEST(EngineBufferTest, RatePermTest);
    EngineBufferScale* m_pScaleVinyl;
    // The keylock engine is configurable, so it could flip flop between
    // ScaleST and ScaleRB during a single callback.
    EngineBufferScale* volatile m_pScaleKeylock;

    // Object used for vinyl-style interpolation scaling of the audio
    EngineBufferScaleLinear* m_pScaleLinear;
    // Objects used for pitch-indep time stretch (key lock) scaling of the audio
    EngineBufferScaleST* m_pScaleST;
    EngineBufferScaleRubberBand* m_pScaleRB;

    // Indicates whether the scaler has changed since the last process()
    bool m_bScalerChanged;
    // Indicates that dependency injection has taken place.
    bool m_bScalerOverride;

    QAtomicInt m_iSeekQueued;
    QAtomicInt m_iSeekPhaseQueued;
    QAtomicInt m_iEnableSyncQueued;
    QAtomicInt m_iSyncModeQueued;
    ControlValueAtomic<double> m_queuedSeekPosition;
    QAtomicPointer<EngineChannel> m_pChannelToCloneFrom;

    // Is true if the previous buffer was silent due to pausing
    QAtomicInt m_iTrackLoading;
    bool m_bPlayAfterLoading;
    // Records the sample rate so we can detect when it changes. Initialized to
    // 0 to guarantee we see a change on the first callback.
    int m_iSampleRate;

    TrackPointer m_pCurrentTrack;
#ifdef __SCALER_DEBUG__
    QFile df;
    QTextStream writer;
#endif

    // Certain operations like seeks and engine changes need to be crossfaded
    // to eliminate clicks and pops.
    CSAMPLE* m_pCrossfadeBuffer;
    bool m_bCrossfadeReady;
    int m_iLastBufferSize;

    QSharedPointer<VisualPlayPosition> m_visualPlayPos;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(EngineBuffer::SeekRequests)

#endif
