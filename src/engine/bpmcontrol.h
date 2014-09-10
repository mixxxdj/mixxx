
// bpmcontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include "controlobject.h"
#include "engine/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "tapfilter.h"

class ControlObject;
class ControlLinPotmeter;
class ControlObjectSlave;
class ControlPushButton;
class EngineBuffer;
class SyncControl;

class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(const char* _group, ConfigObject<ConfigValue>* _config);
    virtual ~BpmControl();

    double getBpm() const;
    double getFileBpm() const { return m_pFileBpm ? m_pFileBpm->get() : 0.0; }
    // When in master sync mode, ratecontrol calls calcSyncedRate to figure out
    // how fast the track should play back.  The rate may be adjusted (ie,
    // not precisely equal to the ratio of the bpms) if the
    // user tweaked the sync position or if the tracks are falling out of sync.
    double calcSyncedRate(double userTweak);
    // Get the phase offset from the specified position.
    double getPhaseOffset(double reference_position);
    double getBeatDistance(double dThisPosition) const;
    double getPreviousSample() const { return m_dPreviousSample; }

    void setCurrentSample(const double dCurrentSample, const double dTotalSamples);
    double process(const double dRate,
                   const double dCurrentSample,
                   const double dTotalSamples,
                   const int iBufferSize);
    void setTargetBeatDistance(double beatDistance);
    void setInstantaneousBpm(double instantaneousBpm);
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
                               double* dpBeatPercentage,
                               const double beatEpsilon=0.0);

    // Returns the shortest change in percentage needed to achieve
    // target_percentage.
    // Example: shortestPercentageChange(0.99, 0.01) == 0.02
    static double shortestPercentageChange(const double& current_percentage,
                                           const double& target_percentage);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotSetEngineBpm(double);
    void slotFileBpmChanged(double);
    void slotAdjustBeatsFaster(double);
    void slotAdjustBeatsSlower(double);
    void slotTranslateBeatsEarlier(double);
    void slotTranslateBeatsLater(double);
    void slotControlPlay(double);
    void slotControlBeatSync(double);
    void slotControlBeatSyncPhase(double);
    void slotControlBeatSyncTempo(double);
    void slotTapFilter(double,int);
    void slotBpmTap(double);
    void slotAdjustRateSlider();
    void slotUpdatedTrackBeats();
    void slotBeatsTranslate(double);

  private:
    SyncMode getSyncMode() const {
        return syncModeFromDouble(m_pSyncMode->get());
    }
    bool syncTempo();
    double calcSyncAdjustment(double my_percentage, bool userTweakingSync);

    friend class SyncControl;

    // ControlObjects that come from EngineBuffer
    ControlObjectSlave* m_pPlayButton;
    ControlObjectSlave* m_pReverseButton;
    ControlObjectSlave* m_pRateSlider;
    ControlObject* m_pQuantize;
    ControlObjectSlave* m_pRateRange;
    ControlObjectSlave* m_pRateDir;

    // ControlObjects that come from LoopingControl
    ControlObjectSlave* m_pLoopEnabled;
    ControlObjectSlave* m_pLoopStartPosition;
    ControlObjectSlave* m_pLoopEndPosition;

    // The current loaded file's detected BPM
    ControlObject* m_pFileBpm;
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

    double m_dPreviousSample;

    // Master Sync objects and values.
    ControlObject* m_pSyncMode;
    ControlObjectSlave* m_pThisBeatDistance;
    double m_dSyncTargetBeatDistance;
    double m_dSyncInstantaneousBpm;
    double m_dLastSyncAdjustment;
    bool m_resetSyncAdjustment;
    double m_dUserOffset;

    TapFilter m_tapFilter;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;

    QString m_sGroup;
};


#endif // BPMCONTROL_H
