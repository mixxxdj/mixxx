#pragma once

#include <gtest/gtest_prod.h>

#include <QAtomicInt>
#include <QMutex>
#include <cfloat>

#include "audio/frame.h"
#include "control/controlvalue.h"
#include "engine/cachingreader/cachingreader.h"
#include "engine/engineobject.h"
#include "engine/sync/syncable.h"
#include "preferences/usersettings.h"
#include "track/bpm.h"
#include "track/track_decl.h"
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
        SEEK_NONE = 0u,
        /// Force an in-phase seek
        SEEK_PHASE = 1u,
        /// Bypass Quantization
        SEEK_EXACT = 2u,
        /// This is an artificial state that happens if an exact seek and a
        /// phase seek are scheduled at the same time.
        SEEK_EXACT_PHASE = SEEK_PHASE | SEEK_EXACT,
        /// #SEEK_PHASE if Quantize enables, otherwise SEEK_EXACT
        SEEK_STANDARD = 4u,
        /// This is an artificial state that happens if a standard seek and a
        /// phase seek are scheduled at the same time.
        SEEK_STANDARD_PHASE = SEEK_STANDARD | SEEK_PHASE,
    };
    Q_DECLARE_FLAGS(SeekRequests, SeekRequest);

    enum KeylockEngine {
        SOUNDTOUCH,
        RUBBERBAND,
        KEYLOCK_ENGINE_COUNT,
    };

    // This value is used to make sure the initial seek after loading a track is
    // not omitted. Therefore this value must be different for 0.0 or any likely
    // value for the main cue
    static constexpr double kInitalSamplePosition = -DBL_MAX;

    EngineBuffer(const QString& group, UserSettingsPointer pConfig,
                 EngineChannel* pChannel, EngineMaster* pMixingEngine);
    virtual ~EngineBuffer();

    void bindWorkers(EngineWorkerScheduler* pWorkerScheduler);

    QString getGroup() const;
    // Return the current rate (not thread-safe)
    double getSpeed() const;
    bool getScratching() const;
    bool isReverse() const;
    /// Returns current bpm value (not thread-safe)
    mixxx::Bpm getBpm() const;
    /// Returns the BPM of the loaded track around the current position (not thread-safe)
    mixxx::Bpm getLocalBpm() const;
    /// Sets a beatloop for the loaded track (not thread safe)
    void setBeatLoop(double startPosition, bool enabled);
    /// Sets a loop for the loaded track (not thread safe)
    void setLoop(double startPosition, double endPositon, bool enabled);
    // Sets pointer to other engine buffer/channel
    void setEngineMaster(EngineMaster*);

    // Queues a new seek position. Use SEEK_EXACT or SEEK_STANDARD as seekType
    void queueNewPlaypos(mixxx::audio::FramePos newpos, enum SeekRequest seekType);
    void requestSyncPhase();
    void requestEnableSync(bool enabled);
    void requestSyncMode(SyncMode mode);
    void requestClonePosition(EngineChannel* pChannel);

    // The process methods all run in the audio callback.
    void process(CSAMPLE* pOut, const int iBufferSize);
    void processSlip(int iBufferSize);
    void postProcess(const int iBufferSize);

    /// Returns the seek position iff a seek is currently queued but not yet
    /// processed. If no seek was queued, and invalid frame position is returned.
    mixxx::audio::FramePos queuedSeekPosition() const;

    bool isTrackLoaded() const;
    TrackPointer getLoadedTrack() const;

    double getExactPlayPos() const;
    double getVisualPlayPos() const;
    double getTrackSamples() const;
    double getUserOffset() const;

    double getRateRatio() const;

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

    // For dependency injection of scalers.
    void setScalerForTest(
            EngineBufferScale* pScaleVinyl,
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

    void setChannelIndex(int channelIndex) {
        m_channelIndex = channelIndex;
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
    void slotKeylockEngineChanged(double);

    void slotEjectTrack(double);

  signals:
    void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void trackLoadFailed(TrackPointer pTrack, const QString& reason);

  private slots:
    void slotTrackLoading();
    void slotTrackLoaded(TrackPointer pTrack,
                         int iSampleRate, int iNumSamples);
    void slotTrackLoadFailed(TrackPointer pTrack,
            const QString& reason);
    // Fired when passthrough mode is enabled or disabled.
    void slotPassthroughChanged(double v);
    void slotUpdatedTrackBeats();

  private:
    struct QueuedSeek {
        mixxx::audio::FramePos position;
        enum SeekRequest seekType;
    };

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
    void doSeekPlayPos(mixxx::audio::FramePos position, enum SeekRequest seekType);

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

    bool updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay);
    void verifyPlay();
    void notifyTrackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void processTrackLocked(CSAMPLE* pOutput,
            const int iBufferSize,
            mixxx::audio::SampleRate sampleRate);

    // Holds the name of the control group
    const QString m_group;
    int m_channelIndex;

    UserSettingsPointer m_pConfig;

    friend class CueControlTest;
    friend class HotcueControlTest;

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
    FRIEND_TEST(EngineSyncTest, UserTweakPreservedInSeek);
    FRIEND_TEST(EngineSyncTest, FollowerUserTweakPreservedInLeaderChange);
    FRIEND_TEST(EngineSyncTest, BeatMapQuantizePlay);
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
    double m_trackSamplesOld;

    // Copy of file sample rate
    mixxx::audio::SampleRate m_trackSampleRateOld;

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

    QAtomicInt m_iSeekPhaseQueued;
    QAtomicInt m_iEnableSyncQueued;
    QAtomicInt m_iSyncModeQueued;
    ControlValueAtomic<QueuedSeek> m_queuedSeek;

    /// Indicates that no seek is queued
    static constexpr QueuedSeek kNoQueuedSeek = {mixxx::audio::kInvalidFramePos, SEEK_NONE};
    QAtomicPointer<EngineChannel> m_pChannelToCloneFrom;

    // Is true if the previous buffer was silent due to pausing
    QAtomicInt m_iTrackLoading;
    bool m_bPlayAfterLoading;
    // Records the sample rate so we can detect when it changes. Initialized to
    // 0 to guarantee we see a change on the first callback.
    mixxx::audio::SampleRate m_sampleRate;

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
