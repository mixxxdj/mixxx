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

#include "util/types.h"
#include "engine/engineobject.h"
#include "engine/sync/syncable.h"
#include "trackinfoobject.h"
#include "configobject.h"
#include "rotary.h"
#include "control/controlvalue.h"

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
class ControlObjectSlave;
class ControlPushButton;
class ControlIndicator;
class ControlObjectThreadMain;
class ControlBeat;
class ControlTTRotary;
class ControlPotmeter;
class CachingReader;
class EngineBufferScale;
class EngineBufferScaleDummy;
class EngineBufferScaleLinear;
class EngineBufferScaleST;
class EngineBufferScaleRubberBand;
class EngineSync;
class EngineWorkerScheduler;
class VisualPlayPosition;
class EngineMaster;

struct Hint;

/**
  *@author Tue and Ken Haste Andersen
*/

// Length of audio beat marks in samples
const int audioBeatMarkLen = 40;

// Temporary buffer length
const int kiTempLength = 200000;

// Rate at which the playpos slider is updated
const int kiPlaypositionUpdateRate = 10; // updates per second
// Number of kiUpdateRates that go by before we update BPM.
const int kiBpmUpdateCnt = 4; // about 2.5 updates per sec

// End of track mode constants
const int TRACK_END_MODE_STOP = 0;
const int TRACK_END_MODE_NEXT = 1;
const int TRACK_END_MODE_LOOP = 2;
const int TRACK_END_MODE_PING = 3;

const int ENGINE_RAMP_DOWN = -1;
const int ENGINE_RAMP_NONE = 0;
const int ENGINE_RAMP_UP = 1;

//const int kiRampLength = 3;

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
        NO_SEEK,
        SEEK_STANDARD,
        SEEK_EXACT,
        SEEK_PHASE
    };

    enum KeylockEngine {
        SOUNDTOUCH,
        RUBBERBAND,
        KEYLOCK_ENGINE_COUNT,
    };

    EngineBuffer(QString _group, ConfigObject<ConfigValue>* _config,
                 EngineChannel* pChannel, EngineMaster* pMixingEngine);
    virtual ~EngineBuffer();

    void bindWorkers(EngineWorkerScheduler* pWorkerScheduler);

    // Add an engine control to the EngineBuffer
    void addControl(EngineControl* pControl);

    // Return the current rate (not thread-safe)
    double getSpeed();
    // Returns current bpm value (not thread-safe)
    double getBpm();
    // Returns the BPM of the loaded track around the current position (not thread-safe)
    double getLocalBpm();
    // Sets pointer to other engine buffer/channel
    void setEngineMaster(EngineMaster*);

    void queueNewPlaypos(double newpos, enum SeekRequest seekType);
    void requestSyncPhase();
    void requestEnableSync(bool enabled);
    void requestSyncMode(SyncMode mode);

    // The process methods all run in the audio callback.
    void process(CSAMPLE* pOut, const int iBufferSize);
    void processSlip(int iBufferSize);
    void postProcess(const int iBufferSize);

    QString getGroup();
    bool isTrackLoaded();
    TrackPointer getLoadedTrack() const;

    double getVisualPlayPos();
    double getTrackSamples();

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

    // For dependency injection of readers.
    //void setReader(CachingReader* pReader);

    // For dependency injection of scalers.
    void setScalerForTest(EngineBufferScale* pScale);

    // For dependency injection of fake tracks, with an optional filebpm value.
    TrackPointer loadFakeTrack(double filebpm = 0);

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
    void slotControlSlip(double);
    void slotKeylockEngineChanged(double);

    // Request that the EngineBuffer load a track. Since the process is
    // asynchronous, EngineBuffer will emit a trackLoaded signal when the load
    // has completed.
    void slotLoadTrack(TrackPointer pTrack, bool play = false);

    void slotEjectTrack(double);

  signals:
    void trackLoaded(TrackPointer pTrack);
    void trackLoadFailed(TrackPointer pTrack, QString reason);
    void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotTrackLoading();
    void slotTrackLoaded(TrackPointer pTrack,
                         int iSampleRate, int iNumSamples);
    void slotTrackLoadFailed(TrackPointer pTrack,
                             QString reason);
    // Fired when passthrough mode is enabled or disabled.
    void slotPassthroughChanged(double v);

  private:
    void enableIndependentPitchTempoScaling(bool bEnable);

    void updateIndicators(double rate, int iBufferSize);

    void hintReader(const double rate);

    void ejectTrack();

    double fractionalPlayposFromAbsolute(double absolutePlaypos);

    void doSeek(double change, enum SeekRequest seekType);

    void clearScale();

    // Reset buffer playpos and set file playpos.
    void setNewPlaypos(double playpos);

    void processSyncRequests();
    void processSeek();

    double updateIndicatorsAndModifyPlay(double v);
    void verifyPlay();

    // Lock for modifying local engine variables that are not thread safe, such
    // as m_engineControls and m_hintList
    QMutex m_engineLock;

    // Holds the name of the control group
    QString m_group;
    ConfigObject<ConfigValue>* m_pConfig;

    LoopingControl* m_pLoopingControl;
    FRIEND_TEST(LoopingControlTest, LoopHalveButton_HalvesLoop);
    FRIEND_TEST(LoopingControlTest, LoopMoveTest);
    FRIEND_TEST(SyncControlTest, TestDetermineBpmMultiplier);
    FRIEND_TEST(EngineSyncTest, HalfDoubleBpmTest);
    FRIEND_TEST(EngineSyncTest, HalfDoubleThenPlay);
    FRIEND_TEST(EngineSyncTest, UserTweakBeatDistance);
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
    QVector<Hint> m_hintList;

    // The current sample to play in the file.
    double m_filepos_play;

    // The previous callback's speed. Used to check if the scaler parameters
    // need updating.
    double m_speed_old;

    // True if the previous callback was scratching.
    bool m_scratching_old;

    // The previous callback's pitch. Used to check if the scaler parameters
    // need updating.
    double m_pitch_old;

    // The previous callback's baserate. Used to check if the scaler parameters
    // need updating.
    double m_baserate_old;

    // Copy of rate_exchange, used to check if rate needs to be updated
    double m_rate_old;

    // Copy of length of file
    long int m_file_length_old;

    // Copy of file sample rate
    int m_file_srate_old;

    // Mutex controlling weather the process function is in pause mode. This happens
    // during seek and loading of a new track
    QMutex m_pause;
    // Used in update of playpos slider
    int m_iSamplesCalculated;
    int m_iUiSlowTick;

    // The location where the track would have been had slip not been engaged
    double m_dSlipPosition;
    // Saved value of rate for slip mode
    double m_dSlipRate;
    // m_slipEnabled is a boolean accessed from multiple threads, so we use an atomic int.
    QAtomicInt m_slipEnabled;
    // m_bSlipEnabledProcessing is only used by the engine processing thread.
    bool m_bSlipEnabledProcessing;
    bool m_bWasKeylocked;


    ControlObject* m_pTrackSamples;
    ControlObject* m_pTrackSampleRate;

    ControlPushButton* m_playButton;
    ControlPushButton* m_playStartButton;
    ControlPushButton* m_stopStartButton;
    ControlPushButton* m_stopButton;

    ControlObject* m_fwdButton;
    ControlObject* m_backButton;
    ControlPushButton* m_pSlipButton;

    ControlObject* m_rateEngine;
    ControlObject* m_visualBpm;
    ControlObject* m_visualKey;
    ControlObject* m_pQuantize;
    ControlObject* m_pMasterRate;
    ControlPotmeter* m_playposSlider;
    ControlObjectSlave* m_pSampleRate;
    ControlObjectSlave* m_pKeylockEngine;
    ControlObjectSlave* m_pPitchControl;
    ControlPushButton* m_pKeylock;
    QScopedPointer<ControlObjectSlave> m_pPassthroughEnabled;

    ControlPushButton* m_pEject;

    // Whether or not to repeat the track when at the end
    ControlPushButton* m_pRepeat;

    // Fwd and back controls, start and end of track control
    ControlPushButton* m_startButton;
    ControlPushButton* m_endButton;

    // Object used to perform waveform scaling (sample rate conversion)
    EngineBufferScale* m_pScale;
    // Object used for linear interpolation scaling of the audio
    EngineBufferScaleLinear* m_pScaleLinear;
    // Object used for pitch-indep time stretch (key lock) scaling of the audio
    EngineBufferScaleST* m_pScaleST;
    EngineBufferScaleRubberBand* m_pScaleRB;
    // The keylock engine is configurable, so it could flip flop between
    // ScaleST and ScaleRB during a single callback.
    EngineBufferScale* volatile m_pScaleKeylock;
    EngineBufferScaleDummy* m_pScaleDummy;
    // Indicates whether the scaler has changed since the last process()
    bool m_bScalerChanged;
    // Indicates that dependency injection has taken place.
    bool m_bScalerOverride;

    QAtomicInt m_iSeekQueued;
    QAtomicInt m_iEnableSyncQueued;
    QAtomicInt m_iSyncModeQueued;
    ControlValueAtomic<double> m_queuedPosition;

    // Holds the last sample value of the previous buffer. This is used when ramping to
    // zero in case of an immediate stop of the playback
    float m_fLastSampleValue[2];
    // Is true if the previous buffer was silent due to pausing
    bool m_bLastBufferPaused;
    QAtomicInt m_iTrackLoading;
    bool m_bPlayAfterLoading;
    float m_fRampValue;
    int m_iRampState;
    //int m_iRampIter;

    TrackPointer m_pCurrentTrack;
#ifdef __SCALER_DEBUG__
    QFile df;
    QTextStream writer;
#endif
    CSAMPLE* m_pDitherBuffer;
    unsigned int m_iDitherBufferReadIndex;
    CSAMPLE* m_pCrossFadeBuffer;
    int m_iCrossFadeSamples;
    int m_iLastBufferSize;

    QSharedPointer<VisualPlayPosition> m_visualPlayPos;
};

#endif
