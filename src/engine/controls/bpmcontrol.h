#pragma once

#include <gtest/gtest_prod.h>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "track/beats.h"
#include "util/tapfilter.h"

class ControlObject;
class ControlLinPotmeter;
class ControlPushButton;
class EngineBuffer;
class SyncControl;

/// BpmControl is an EngineControl that manages the bpm and beat distance of
/// tracks.  It understands the tempo of the underlying track and the musical
/// position of the playhead.
class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(const QString& group, UserSettingsPointer pConfig);
    ~BpmControl() override;

    double getBpm() const;
    double getLocalBpm() const { return m_pLocalBpm ? m_pLocalBpm->get() : 0.0; }
    // When in master sync mode, ratecontrol calls calcSyncedRate to figure out
    // how fast the track should play back.  The returned rate is usually just
    // the correct pitch to match bpms.  The usertweak argument represents
    // how much the user is nudging the pitch to get two tracks into sync, and
    // that value is added to the rate by bpmcontrol.  The rate may be
    // further adjusted if bpmcontrol discovers that the tracks have fallen
    // out of sync.
    double calcSyncedRate(double userTweak);
    // Get the phase offset from the specified position.
    double getNearestPositionInPhase(double dThisPosition, bool respectLoops, bool playing);
    double getBeatMatchPosition(double dThisPosition, bool respectLoops, bool playing);
    double getPhaseOffset(double dThisPosition);
    /// getBeatDistance is adjusted to include the user offset so it's
    /// transparent to other decks.
    double getBeatDistance(double dThisPosition) const;
    double getUserOffset() const {
        return m_dUserOffset.getValue();
    }

    void setTargetBeatDistance(double beatDistance);
    void updateInstantaneousBpm(double instantaneousBpm);
    void resetSyncAdjustment();
    double updateLocalBpm();
    /// updateBeatDistance is adjusted to include the user offset so
    /// it's transparent to other decks.
    double updateBeatDistance();

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

    // Calculates contextual information about beats: the previous beat, the
    // next beat, the current beat length, and the beat ratio (how far dPosition
    // lies within the current beat). Returns false if a previous or next beat
    // does not exist. NULL arguments are safe and ignored.
    static bool getBeatContext(const mixxx::BeatsPointer& pBeats,
            const double dPosition,
            double* dpPrevBeat,
            double* dpNextBeat,
            double* dpBeatLength,
            double* dpBeatPercentage);

    // Alternative version that works if the next and previous beat positions
    // are already known.
    static bool getBeatContextNoLookup(
                               const double dPosition,
                               const double dPrevBeat,
                               const double dNextBeat,
                               double* dpBeatLength,
                               double* dpBeatPercentage);

    // Returns the shortest change in percentage needed to achieve
    // target_percentage.
    // Example: shortestPercentageChange(0.99, 0.01) == 0.02
    static double shortestPercentageChange(const double& current_percentage,
                                           const double& target_percentage);
    double getRateRatio() const;
    void notifySeek(double dNewPlaypos) override;
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private slots:
    void slotAdjustBeatsFaster(double);
    void slotAdjustBeatsSlower(double);
    void slotTranslateBeatsEarlier(double);
    void slotTranslateBeatsLater(double);
    void slotControlBeatSync(double);
    void slotControlBeatSyncPhase(double);
    void slotControlBeatSyncTempo(double);
    void slotTapFilter(double,int);
    void slotBpmTap(double);
    void slotUpdateRateSlider(double v = 0.0);
    void slotUpdateEngineBpm(double v = 0.0);
    void slotBeatsTranslate(double);
    void slotBeatsTranslateMatchAlignment(double);

  private:
    SyncMode getSyncMode() const {
        return syncModeFromDouble(m_pSyncMode->get());
    }
    inline bool isSynchronized() const {
        return toSynchronized(getSyncMode());
    }
    bool syncTempo();
    double calcSyncAdjustment(bool userTweakingSync);
    void adjustBeatsBpm(double deltaBpm);

    friend class SyncControl;

    // ControlObjects that come from EngineBuffer
    ControlProxy* m_pPlayButton;
    QAtomicInt m_oldPlayButton;
    ControlProxy* m_pReverseButton;
    ControlProxy* m_pRateRatio;
    ControlObject* m_pQuantize;

    // ControlObjects that come from QuantizeControl
    QScopedPointer<ControlProxy> m_pNextBeat;
    QScopedPointer<ControlProxy> m_pPrevBeat;

    // ControlObjects that come from LoopingControl
    ControlProxy* m_pLoopEnabled;
    ControlProxy* m_pLoopStartPosition;
    ControlProxy* m_pLoopEndPosition;

    // The average bpm around the current playposition;
    ControlObject* m_pLocalBpm;
    ControlPushButton* m_pAdjustBeatsFaster;
    ControlPushButton* m_pAdjustBeatsSlower;
    ControlPushButton* m_pTranslateBeatsEarlier;
    ControlPushButton* m_pTranslateBeatsLater;

    // The current effective BPM of the engine
    ControlLinPotmeter* m_pEngineBpm;

    // Used for bpm tapping from GUI and MIDI
    ControlPushButton* m_pButtonTap;

    // Button for sync'ing with the other EngineBuffer
    ControlPushButton* m_pButtonSync;
    ControlPushButton* m_pButtonSyncPhase;
    ControlPushButton* m_pButtonSyncTempo;

    // Button that translates the beats so the nearest beat is on the current
    // playposition.
    ControlPushButton* m_pTranslateBeats;
    // Button that translates beats to match another playing deck
    ControlPushButton* m_pBeatsTranslateMatchAlignment;

    ControlProxy* m_pThisBeatDistance;
    ControlValueAtomic<double> m_dSyncTargetBeatDistance;
    // The user offset is a beat distance percentage value that the user has tweaked a deck
    // to bring it in sync with the other decks. This value is added to the reported beat
    // distance to get the virtual beat distance used for sync.
    ControlValueAtomic<double> m_dUserOffset;
    QAtomicInt m_resetSyncAdjustment;
    ControlProxy* m_pSyncMode;

    TapFilter m_tapFilter; // threadsafe

    // used in the engine thread only
    double m_dSyncInstantaneousBpm;
    double m_dLastSyncAdjustment;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;

    FRIEND_TEST(EngineSyncTest, UserTweakPreservedInSeek);
    FRIEND_TEST(EngineSyncTest, FollowerUserTweakPreservedInMasterChange);
};
