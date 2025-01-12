#pragma once

#include <gtest/gtest_prod.h>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "track/beats.h"
#include "util/tapfilter.h"

class ControlEncoder;
class ControlLinPotmeter;
class ControlPushButton;

/// BpmControl is an EngineControl that manages the bpm and beat distance of
/// tracks.  It understands the tempo of the underlying track and the musical
/// position of the playhead.
class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(const QString& group, UserSettingsPointer pConfig);

    mixxx::Bpm getBpm() const;
    mixxx::Bpm getLocalBpm() const {
        return m_pLocalBpm ? mixxx::Bpm(m_pLocalBpm->get()) : mixxx::Bpm();
    }

    // When in sync lock mode, ratecontrol calls calcSyncedRate to figure out
    // how fast the track should play back.  The returned rate is usually just
    // the correct pitch to match bpms.  The usertweak argument represents
    // how much the user is nudging the pitch to get two tracks into sync, and
    // that value is added to the rate by bpmcontrol.  The rate may be
    // further adjusted if bpmcontrol discovers that the tracks have fallen
    // out of sync.
    double calcSyncedRate(double userTweak);
    // Get the phase offset from the specified position.
    mixxx::audio::FramePos getNearestPositionInPhase(
            mixxx::audio::FramePos thisPosition,
            bool respectLoops,
            bool playing);
    mixxx::audio::FramePos getBeatMatchPosition(
            mixxx::audio::FramePos thisPosition,
            bool respectLoops,
            bool playing);
    double getPhaseOffset(mixxx::audio::FramePos thisPosition);
    /// getBeatDistance is adjusted to include the user offset so it's
    /// transparent to other decks.
    double getBeatDistance(mixxx::audio::FramePos thisPosition) const;
    double getUserOffset() const {
        return m_dUserOffset.getValue();
    }

    void setTargetBeatDistance(double beatDistance);
    void updateInstantaneousBpm(double instantaneousBpm);
    void resetSyncAdjustment();
    mixxx::Bpm updateLocalBpm();
    /// Updates the beat distance based on the current play position.
    /// This override is called on every engine callback to update the
    /// beatposition based on the new current playposition.
    double updateBeatDistance();
    /// Updates the beat distance based on the provided play position. This
    /// override is used for seeks.
    double updateBeatDistance(mixxx::audio::FramePos playpos);

    void collectFeatures(GroupFeatureState* pGroupFeatures, double speed) const;

    // Calculates contextual information about beats: the previous beat, the
    // next beat, the current beat length, and the beat ratio (how far dPosition
    // lies within the current beat). Returns false if a previous or next beat
    // does not exist. NULL arguments are safe and ignored.
    static bool getBeatContext(const mixxx::BeatsPointer& pBeats,
            mixxx::audio::FramePos position,
            mixxx::audio::FramePos* pPrevBeatPosition,
            mixxx::audio::FramePos* pNextBeatPosition,
            mixxx::audio::FrameDiff_t* pBeatLengthFrames,
            double* pBeatPercentage);

    // Alternative version that works if the next and previous beat positions
    // are already known.
    static bool getBeatContextNoLookup(mixxx::audio::FramePos position,
            mixxx::audio::FramePos prevBeatPosition,
            mixxx::audio::FramePos nextBeatPosition,
            mixxx::audio::FrameDiff_t* pBeatLengthFrames,
            double* pBeatPercentage);

    // Returns the shortest change in percentage needed to achieve
    // target_percentage.
    // Example: shortestPercentageChange(0.99, 0.01) == 0.02
    static double shortestPercentageChange(const double& current_percentage,
                                           const double& target_percentage);
    double getRateRatio() const;
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;
    void trackBpmLockChanged(bool locked);
    void notifySeek(mixxx::audio::FramePos position) override;

  private slots:
    void slotAdjustBeatsFaster(double);
    void slotAdjustBeatsSlower(double);
    void slotTranslateBeatsEarlier(double);
    void slotTranslateBeatsLater(double);
    void slotTranslateBeatsMove(double);

    void slotBpmTap(double value);
    void slotBpmTapFilter(double averageLength, int numSamples);
    void slotTempoTap(double value);
    void slotTempoTapFilter(double averageLength, int numSamples);

    void slotUpdateRateSlider(double v = 0.0);
    void slotUpdateEngineBpm(double v = 0.0);
    void slotBeatsTranslate(double);
    void slotBeatsTranslateMatchAlignment(double);
    void slotToggleBpmLock(double);
    void slotBeatsUndoAdjustment(double value);

  private:
    SyncMode getSyncMode() const {
        return syncModeFromDouble(m_pSyncMode.get());
    }
    inline bool isSynchronized() const {
        return toSynchronized(getSyncMode());
    }
    double calcSyncAdjustment(bool userTweakingSync);
    void adjustBeatsBpm(double deltaBpm);
    void slotScaleBpm(mixxx::Beats::BpmScale bpmScale);

    friend class SyncControl;

    // ControlObjects that come from EngineBuffer
    PollingControlProxy m_pReverseButton;
    std::unique_ptr<ControlProxy> m_pRateRatio;
    PollingControlProxy m_pQuantize;

    // ControlObjects that come from QuantizeControl
    PollingControlProxy m_pNextBeat;
    PollingControlProxy m_pPrevBeat;

    // ControlObjects that come from LoopingControl
    PollingControlProxy m_pLoopEnabled;
    PollingControlProxy m_pLoopStartPosition;
    PollingControlProxy m_pLoopEndPosition;

    // The average bpm around the current playposition;
    std::unique_ptr<ControlObject> m_pLocalBpm;
    std::unique_ptr<ControlPushButton> m_pAdjustBeatsFaster;
    std::unique_ptr<ControlPushButton> m_pAdjustBeatsSlower;
    std::unique_ptr<ControlPushButton> m_pTranslateBeatsEarlier;
    std::unique_ptr<ControlPushButton> m_pTranslateBeatsLater;
    std::unique_ptr<ControlEncoder> m_pTranslateBeatsMove;
    std::unique_ptr<ControlPushButton> m_pBeatsUndo;
    std::unique_ptr<ControlObject> m_pBeatsUndoPossible;

    std::unique_ptr<ControlPushButton> m_pBeatsHalve;
    std::unique_ptr<ControlPushButton> m_pBeatsTwoThirds;
    std::unique_ptr<ControlPushButton> m_pBeatsThreeFourths;
    std::unique_ptr<ControlPushButton> m_pBeatsFourThirds;
    std::unique_ptr<ControlPushButton> m_pBeatsThreeHalves;
    std::unique_ptr<ControlPushButton> m_pBeatsDouble;

    std::unique_ptr<ControlPushButton> m_pBpmLock;

    // The current effective BPM of the engine
    std::unique_ptr<ControlLinPotmeter> m_pEngineBpm;

    // Used for bpm tapping from GUI and MIDI
    std::unique_ptr<ControlPushButton> m_pBpmTap;   // File BPM
    std::unique_ptr<ControlPushButton> m_pTempoTap; // Enigne BPM (playback rate)

    // Button that translates the beats so the nearest beat is on the current
    // playposition.
    std::unique_ptr<ControlPushButton> m_pTranslateBeats;
    // Button that translates beats to match another playing deck
    std::unique_ptr<ControlPushButton> m_pBeatsTranslateMatchAlignment;

    PollingControlProxy m_pThisBeatDistance;
    ControlValueAtomic<double> m_dSyncTargetBeatDistance;
    // The user offset is a beat distance percentage value that the user has tweaked a deck
    // to bring it in sync with the other decks. This value is added to the reported beat
    // distance to get the virtual beat distance used for sync.
    ControlValueAtomic<double> m_dUserOffset;
    QAtomicInt m_resetSyncAdjustment;
    PollingControlProxy m_pSyncMode;

    TapFilter m_bpmTapFilter;   // threadsafe
    TapFilter m_tempoTapFilter; // threadsafe

    // used in the engine thread only
    double m_dSyncInstantaneousBpm;
    double m_dLastSyncAdjustment;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;

    FRIEND_TEST(EngineSyncTest, UserTweakPreservedInSeek);
    FRIEND_TEST(EngineSyncTest, FollowerUserTweakPreservedInLeaderChange);
    FRIEND_TEST(EngineSyncTest, FollowerUserTweakPreservedInSyncDisable);
};
