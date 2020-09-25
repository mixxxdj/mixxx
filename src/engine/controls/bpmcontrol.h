#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include <gtest/gtest_prod.h>

#include "control/controllinpotmeter.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "util/parented_ptr.h"
#include "util/tapfilter.h"

class ControlObject;
class ControlProxy;
class EngineBuffer;
class SyncControl;

/// BpmControl is an EngineControl that manages the bpm and beat distance of
/// tracks.  It understands the tempo of the underlying track and the musical
/// position of the playhead.
class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(const QString& group, UserSettingsPointer pConfig);
    ~BpmControl() override = default;

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
    mixxx::FramePos getNearestPositionInPhase(
            mixxx::FramePos thisPosition, bool respectLoops, bool playing);
    mixxx::FramePos getBeatMatchPosition(
            mixxx::FramePos thisPosition, bool respectLoops, bool playing);
    mixxx::FrameDiff_t getPhaseOffset(mixxx::FramePos thisPosition);
    /// getBeatDistance is adjusted to include the user offset so it's
    /// transparent to other decks.
    double getBeatDistance(double dThisPosition) const;

    void setTargetBeatDistance(double beatDistance);
    void setInstantaneousBpm(double instantaneousBpm);
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
    static bool getBeatContext(mixxx::BeatsPointer pBeats,
            mixxx::FramePos position,
            mixxx::FramePos* pPrevBeat,
            mixxx::FramePos* pNextBeat,
            mixxx::FrameDiff_t* dpBeatLength,
            double* dpBeatPercentage);

    // Alternative version that works if the next and previous beat positions
    // are already known.
    static bool getBeatContextNoLookup(
            mixxx::FramePos position,
            mixxx::FramePos pPrevBeat,
            mixxx::FramePos pNextBeat,
            mixxx::FrameDiff_t* dpBeatLength,
            double* dpBeatPercentage);

    // Returns the shortest change in percentage needed to achieve
    // target_percentage.
    // Example: shortestPercentageChange(0.99, 0.01) == 0.02
    static double shortestPercentageChange(double current_percentage,
            double target_percentage);
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

    friend class SyncControl;

    // ControlObjects that come from EngineBuffer
    parented_ptr<ControlProxy> m_pPlayButton;
    QAtomicInt m_oldPlayButton;
    parented_ptr<ControlProxy> m_pReverseButton;
    parented_ptr<ControlProxy> m_pRateRatio;
    ControlObject* m_pQuantize;

    // ControlObjects that come from QuantizeControl
    parented_ptr<ControlProxy> m_pNextBeat;
    parented_ptr<ControlProxy> m_pPrevBeat;
    parented_ptr<ControlProxy> m_pClosestBeat;

    // ControlObjects that come from LoopingControl
    parented_ptr<ControlProxy> m_pLoopEnabled;
    parented_ptr<ControlProxy> m_pLoopStartPosition;
    parented_ptr<ControlProxy> m_pLoopEndPosition;

    // The average bpm around the current playposition;
    std::unique_ptr<ControlObject> m_pLocalBpm;
    std::unique_ptr<ControlPushButton> m_pAdjustBeatsFaster;
    std::unique_ptr<ControlPushButton> m_pAdjustBeatsSlower;
    std::unique_ptr<ControlPushButton> m_pTranslateBeatsEarlier;
    std::unique_ptr<ControlPushButton> m_pTranslateBeatsLater;

    // The current effective BPM of the engine
    std::unique_ptr<ControlLinPotmeter> m_pEngineBpm;

    // Used for bpm tapping from GUI and MIDI
    std::unique_ptr<ControlPushButton> m_pButtonTap;

    // Button for sync'ing with the other EngineBuffer
    std::unique_ptr<ControlPushButton> m_pButtonSync;
    std::unique_ptr<ControlPushButton> m_pButtonSyncPhase;
    std::unique_ptr<ControlPushButton> m_pButtonSyncTempo;

    // Button that translates the beats so the nearest beat is on the current
    // playposition.
    std::unique_ptr<ControlPushButton> m_pTranslateBeats;
    // Button that translates beats to match another playing deck
    std::unique_ptr<ControlPushButton> m_pBeatsTranslateMatchAlignment;
    // Button to set the nearest beat as a downbeat
    std::unique_ptr<ControlPushButton> m_pBeatsSetDownbeat;

    parented_ptr<ControlProxy> m_pThisBeatDistance;
    ControlValueAtomic<double> m_dSyncTargetBeatDistance;
    ControlValueAtomic<double> m_dUserOffset;
    QAtomicInt m_resetSyncAdjustment;
    parented_ptr<ControlProxy> m_pSyncMode;

    TapFilter m_tapFilter; // threadsafe

    // used in the engine thread only
    double m_dSyncInstantaneousBpm;
    double m_dLastSyncAdjustment;
    bool m_dUserTweakingSync;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;

    FRIEND_TEST(EngineSyncTest, UserTweakBeatDistance);
    FRIEND_TEST(EngineSyncTest, UserTweakPreservedInSeek);
};

#endif // BPMCONTROL_H
