#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include <gtest/gtest_prod.h>

#include "control/controlobject.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "util/tapfilter.h"

class ControlObject;
class ControlLinPotmeter;
class ControlProxy;
class ControlPushButton;
class EngineBuffer;
class SyncControl;

class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(QString group, UserSettingsPointer pConfig);
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
    double getPhaseOffset(double dThisPosition);
    double getBeatDistance(double dThisPosition) const;

    void setTargetBeatDistance(double beatDistance);
    void setInstantaneousBpm(double instantaneousBpm);
    void resetSyncAdjustment();
    double updateLocalBpm();
    double updateBeatDistance();

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

    // Calculates contextual information about beats: the previous beat, the
    // next beat, the current beat length, and the beat ratio (how far dPosition
    // lies within the current beat). Returns false if a previous or next beat
    // does not exist. NULL arguments are safe and ignored.
    static bool getBeatContext(const BeatsPointer& pBeats,
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
    void trackLoaded(TrackPointer pNewTrack) override;

  private slots:
    void slotFileBpmChanged(double);
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
    void slotUpdatedTrackBeats();
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
    double calcSyncAdjustment(double my_percentage, bool userTweakingSync);
    double calcRateRatio() const;

    friend class SyncControl;

    // ControlObjects that come from EngineBuffer
    ControlProxy* m_pPlayButton;
    QAtomicInt m_oldPlayButton;
    ControlProxy* m_pReverseButton;
    ControlProxy* m_pRateSlider;
    ControlObject* m_pQuantize;
    ControlProxy* m_pRateRange;
    ControlProxy* m_pRateDir;

    // ControlObjects that come from QuantizeControl
    QScopedPointer<ControlProxy> m_pNextBeat;
    QScopedPointer<ControlProxy> m_pPrevBeat;
    QScopedPointer<ControlProxy> m_pClosestBeat;

    // ControlObjects that come from LoopingControl
    ControlProxy* m_pLoopEnabled;
    ControlProxy* m_pLoopStartPosition;
    ControlProxy* m_pLoopEndPosition;

    // The current loaded file's detected BPM
    ControlObject* m_pFileBpm;
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
    ControlValueAtomic<double> m_dUserOffset;
    QAtomicInt m_resetSyncAdjustment;
    ControlProxy* m_pSyncMode;

    TapFilter m_tapFilter; // threadsafe

    // used in the engine thread only
    double m_dSyncInstantaneousBpm;
    double m_dLastSyncAdjustment;

    // objects below are written from an engine worker thread
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;

    const QString m_sGroup;

    FRIEND_TEST(EngineSyncTest, UserTweakBeatDistance);
};


#endif // BPMCONTROL_H
