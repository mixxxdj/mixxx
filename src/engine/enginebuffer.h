#pragma once

#include <gtest/gtest_prod.h>

#include <QAtomicInt>
#include <QMutex>
#include <initializer_list>

#include "audio/frame.h"
#include "audio/types.h"
#include "control/controlvalue.h"
#include "control/pollingcontrolproxy.h"
#include "engine/cachingreader/cachingreader.h"
#include "engine/engineobject.h"
#include "engine/slipmodestate.h"
#include "engine/sync/syncable.h"
#include "preferences/usersettings.h"
#include "track/bpm.h"
#include "track/track_decl.h"
#include "util/types.h"

#ifdef __RUBBERBAND__
#include "engine/bufferscalers/enginebufferscalerubberband.h"
#endif

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
class ControlPotmeter;
class EngineBufferScale;
class EngineBufferScaleLinear;
class EngineBufferScaleST;
class EngineSync;
class EngineWorkerScheduler;
class VisualPlayPosition;
class EngineMixer;

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
        SEEK_NONE = 0,
        /// Force an in-phase seek
        SEEK_PHASE = 1 << 0,
        /// Bypass Quantization
        SEEK_EXACT = 1 << 1,
        /// This is an artificial state that happens if an exact seek and a
        /// phase seek are scheduled at the same time.
        SEEK_EXACT_PHASE = SEEK_PHASE | SEEK_EXACT,
        /// #SEEK_PHASE if Quantize enables, otherwise SEEK_EXACT
        SEEK_STANDARD = 1 << 2,
        /// This is an artificial state that happens if a standard seek and a
        /// phase seek are scheduled at the same time.
        SEEK_STANDARD_PHASE = SEEK_STANDARD | SEEK_PHASE,
        /// #SEEK_EXACT to the other deck position
        SEEK_CLONE = 1 << 3
    };
    Q_DECLARE_FLAGS(SeekRequests, SeekRequest);

    // This enum is also used in mixxx.cfg
    // Don't remove or swap values to keep backward compatibility
    enum class KeylockEngine {
        SoundTouch = 0,
#ifdef __RUBBERBAND__
        RubberBandFaster = 1,
        RubberBandFiner = 2,
#endif
    };

    // intended for iteration over the KeylockEngine enum
    constexpr static std::initializer_list<KeylockEngine> kKeylockEngines = {
            KeylockEngine::SoundTouch,
#ifdef __RUBBERBAND__
            KeylockEngine::RubberBandFaster,
            KeylockEngine::RubberBandFiner
#endif
    };

    EngineBuffer(const QString& group,
            UserSettingsPointer pConfig,
            EngineChannel* pChannel,
            EngineMixer* pMixingEngine,
            mixxx::audio::ChannelCount maxSupportedChannel);
    virtual ~EngineBuffer();

    void bindWorkers(EngineWorkerScheduler* pWorkerScheduler);

    QString getGroup() const;
    // Return the current rate (not thread-safe)
    double getSpeed() const;
    mixxx::audio::ChannelCount getChannelCount() const {
        return m_channelCount;
    }
    bool getScratching() const;
    bool isReverse() const;
    /// Returns current bpm value (not thread-safe)
    mixxx::Bpm getBpm() const;
    /// Returns the BPM of the loaded track around the current position (not thread-safe)
    mixxx::Bpm getLocalBpm() const;
    /// Sets a beatloop for the loaded track (not thread safe)
    void setBeatLoop(mixxx::audio::FramePos startPosition, bool enabled);
    /// Sets a loop for the loaded track (not thread safe)
    void setLoop(mixxx::audio::FramePos startPosition,
            mixxx::audio::FramePos endPositon,
            bool enabled);
    // Sets pointer to other engine buffer/channel
    void setEngineMixer(EngineMixer*);

    // Queues a new seek position. Use SEEK_EXACT or SEEK_STANDARD as seekType
    void queueNewPlaypos(mixxx::audio::FramePos newpos, enum SeekRequest seekType);
    void requestSyncPhase();
    void requestEnableSync(bool enabled);
    void requestSyncMode(SyncMode mode);

    // The process methods all run in the audio callback.
    void process(CSAMPLE* pOut, const std::size_t bufferSize) override;
    void processSlip(std::size_t bufferSize);
    void postProcessLocalBpm();
    void postProcess(const std::size_t bufferSize);

    /// Returns the seek position iff a seek is currently queued but not yet
    /// processed. If no seek was queued, and invalid frame position is returned.
    mixxx::audio::FramePos queuedSeekPosition() const;

    bool isTrackLoaded() const;
    TrackPointer getLoadedTrack() const;
    void ejectTrack();

    mixxx::audio::FramePos getExactPlayPos() const;
    double getVisualPlayPos() const;
    mixxx::audio::FramePos getTrackEndPosition() const;
    void setTrackEndPosition(mixxx::audio::FramePos position);
    double getUserOffset() const;

    double getRateRatio() const;

    void collectFeatures(GroupFeatureState* pGroupFeatures) const override;

    // For dependency injection of scalers.
    void setScalerForTest(
            EngineBufferScale* pScaleVinyl,
            EngineBufferScale* pScaleKeylock);

    // For injection of fake tracks.
    void loadFakeTrack(TrackPointer pTrack, bool bPlay);

    static QString getKeylockEngineName(KeylockEngine engine) {
        switch (engine) {
        case KeylockEngine::SoundTouch:
            return tr("Soundtouch (faster)");
#ifdef __RUBBERBAND__
        case KeylockEngine::RubberBandFaster:
            return tr("Rubberband (better)");
        case KeylockEngine::RubberBandFiner:
            if (EngineBufferScaleRubberBand::isEngineFinerAvailable()) {
                return tr("Rubberband R3 (near-hi-fi quality)");
            }
            [[fallthrough]];
#endif
        default:
#ifdef __RUBBERBAND__
            return tr("Unknown, using Rubberband (better)");
#else
            return tr("Unknown, using Soundtouch");
#endif
        }
    }

    static bool isKeylockEngineAvailable(KeylockEngine engine) {
        switch (engine) {
        case KeylockEngine::SoundTouch:
            return true;
#ifdef __RUBBERBAND__
        case KeylockEngine::RubberBandFaster:
            return true;
        case KeylockEngine::RubberBandFiner:
            return EngineBufferScaleRubberBand::isEngineFinerAvailable();
#endif
        default:
            return false;
        }
    }

    constexpr static KeylockEngine defaultKeylockEngine() {
#ifdef __RUBBERBAND__
        return KeylockEngine::RubberBandFaster;
#else
        return KeylockEngine::SoundTouch;
#endif
    }

    // Request that the EngineBuffer load a track. Since the process is
    // asynchronous, EngineBuffer will emit a trackLoaded signal when the load
    // has completed.
#ifdef __STEM__
    void loadTrack(TrackPointer pTrack,
            mixxx::StemChannelSelection stemMask,
            bool play,
            EngineChannel* pChannelToCloneFrom);
#else
    void loadTrack(TrackPointer pTrack,
            bool play,
            EngineChannel* pChannelToCloneFrom);
#endif

    void setChannelIndex(int channelIndex) {
        m_channelIndex = channelIndex;
    }

    void seekAbs(mixxx::audio::FramePos);
    void seekExact(mixxx::audio::FramePos);

    void verifyPlay();

    void slipQuitAndAdopt();

  public slots:
    void slotControlPlayRequest(double);
    void slotControlPlayFromStart(double);
    void slotControlJumpToStartAndStop(double);
    void slotControlStop(double);
    void slotControlStart(double);
    void slotControlEnd(double);
    void slotControlSeek(double);
    void slotKeylockEngineChanged(double);

  signals:
    void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void trackLoadFailed(TrackPointer pTrack, const QString& reason);

  private slots:
    void slotTrackLoading();
    void slotTrackLoaded(
            TrackPointer pTrack,
            mixxx::audio::SampleRate trackSampleRate,
            mixxx::audio::ChannelCount trackChannelCount,
            mixxx::audio::FramePos trackNumFrame);
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
            const std::size_t bufferSize);

    void updateIndicators(double rate, std::size_t bufferSize);

    void hintReader(const double rate);

    double fractionalPlayposFromAbsolute(mixxx::audio::FramePos position);

    void doSeekFractional(double fractionalPos, enum SeekRequest seekType);
    void doSeekPlayPos(mixxx::audio::FramePos position, enum SeekRequest seekType);

    // Read one buffer from the current scaler into the crossfade buffer.  Used
    // for transitioning from one scaler to another, or reseeking a scaler
    // to prevent pops.
    void readToCrossfadeBuffer(const std::size_t bufferSize);

    // Reset buffer playpos and set file playpos.
    void setNewPlaypos(mixxx::audio::FramePos playpos);

    void processSyncRequests();
    void processSeek(bool paused);
    // For debugging / testing -- returns true if the previous buffer call resulted in a seek.
    FRIEND_TEST(EngineSyncTest, FollowerUserTweakPreservedInSyncDisable);
    bool previousBufferSeek() const {
        return m_previousBufferSeek;
    }
    bool updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay);
    void notifyTrackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void processTrackLocked(CSAMPLE* pOutput,
            const std::size_t bufferSize,
            mixxx::audio::SampleRate sampleRate);

    // Holds the name of the control group
    const QString m_group;
    int m_channelIndex;

    UserSettingsPointer m_pConfig;

    friend class CueControlTest;
    friend class HotcueControlTest;
    friend class LoopingControlTest;

    LoopingControl* m_pLoopingControl; // used for tests
    FRIEND_TEST(LoopingControlTest, LoopScale_HalvesLoop);
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
    FRIEND_TEST(CueControlTest, SeekOnSetCueCDJ);
    FRIEND_TEST(CueControlTest, SeekOnSetCuePlay);
    CueControl* m_pCueControl;

    QList<EngineControl*> m_engineControls;

    // The read ahead manager for EngineBufferScale's that need to read ahead
    ReadAheadManager* m_pReadAheadManager;

    // The reader used to read audio files
    CachingReader* m_pReader;

    // List of hints to provide to the CachingReader
    HintVector m_hintList;

    // The current frame to play in the file.
    mixxx::audio::FramePos m_playPos;

    // The previous callback's speed. Used to check if the scaler parameters
    // need updating.
    double m_speed_old;

    // The actual speed is the speed calculated from buffer start and end position
    // It can differ form m_speed_old which holds the requested speed.
    // It is the average of one fuffer, in case speed ramping is applied in the scalers.
    double m_actual_speed;

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
    mixxx::audio::FramePos m_trackEndPositionOld;

    // Copy of file sample rate
    mixxx::audio::SampleRate m_trackSampleRateOld;

    // Mutex controlling whether the process function is in pause mode. This happens
    // during seek and loading of a new track
    QMutex m_pause;
    // Used in update of playpos slider
    std::size_t m_samplesSinceLastIndicatorUpdate;

    // The location where the track would have been had slip not been engaged
    mixxx::audio::FramePos m_slipPos;
    // Saved value of rate for slip mode
    double m_dSlipRate;
    // m_bSlipEnabledProcessing is only used by the engine processing thread.
    bool m_bSlipEnabledProcessing;

    SlipModeState m_slipModeState;

    // Track samples are always given assuming a stereo track
    ControlObject* m_pTrackSamples;
    ControlObject* m_pTrackSampleRate;

    ControlPushButton* m_playButton;
    ControlPushButton* m_playStartButton;
    ControlPushButton* m_stopStartButton;
    ControlPushButton* m_stopButton;

    ControlPushButton* m_pSlipButton;

    PollingControlProxy m_quantize;
    ControlPotmeter* m_playposSlider;
    ControlProxy* m_pSampleRate;
    ControlProxy* m_pKeylockEngine;
    ControlPushButton* m_pKeylock;
    ControlProxy* m_pReplayGain;

    // This ControlProxys is created as parent to this and deleted by
    // the Qt object tree. This helps that they are deleted by the creating
    // thread, which is required to avoid segfaults.
    ControlProxy* m_pPassthroughEnabled;

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
#ifdef __RUBBERBAND__
    EngineBufferScaleRubberBand* m_pScaleRB;
#endif

    // Indicates whether the scaler has changed since the last process()
    bool m_bScalerChanged;
    // Indicates that dependency injection has taken place.
    bool m_bScalerOverride;

    QAtomicInt m_iSeekPhaseQueued;
    QAtomicInt m_iEnableSyncQueued;
    QAtomicInt m_iSyncModeQueued;
    ControlValueAtomic<QueuedSeek> m_queuedSeek;
    bool m_previousBufferSeek = false;

    QAtomicInt m_slipQuitAndAdopt;
    /// Indicates that no seek is queued
    static constexpr QueuedSeek kNoQueuedSeek = {mixxx::audio::kInvalidFramePos, SEEK_NONE};
    /// indicates a clone seek on a bosition from another deck
    static constexpr QueuedSeek kCloneSeek = {mixxx::audio::kInvalidFramePos, SEEK_CLONE};
    QAtomicPointer<EngineChannel> m_pChannelToCloneFrom;

    // Is true if the previous buffer was silent due to pausing
    QAtomicInt m_iTrackLoading;
    bool m_bPlayAfterLoading;
    // Records the sample rate so we can detect when it changes. Initialized to
    // 0 to guarantee we see a change on the first callback.
    mixxx::audio::SampleRate m_sampleRate;

    // The current channel count of the loaded track
    mixxx::audio::ChannelCount m_channelCount;

    TrackPointer m_pCurrentTrack;
#ifdef __SCALER_DEBUG__
    QFile df;
    QTextStream writer;
#endif

    // Certain operations like seeks and engine changes need to be crossfaded
    // to eliminate clicks and pops.
    CSAMPLE* m_pCrossfadeBuffer;
    bool m_bCrossfadeReady;
    std::size_t m_lastBufferSize;

    QSharedPointer<VisualPlayPosition> m_visualPlayPos;
};

Q_DECLARE_METATYPE(EngineBuffer::KeylockEngine)
Q_DECLARE_OPERATORS_FOR_FLAGS(EngineBuffer::SeekRequests)
