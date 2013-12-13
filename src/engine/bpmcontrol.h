
// bpmcontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include "controlobject.h"
#include "engine/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "tapfilter.h"

class ControlObject;
class ControlPushButton;
class EngineBuffer;
class ControlObjectSlave;

class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(const char* _group, ConfigObject<ConfigValue>* _config);
    virtual ~BpmControl();

    double getBpm() const;
    double getFileBpm() const { return m_pFileBpm ? m_pFileBpm->get() : 0.0; }
    double getSyncAdjustment(bool userTweakingSync);
    double getSyncedRate() const;
    // Get the phase offset from the specified position.
    double getPhaseOffset(double reference_position);

    void setCurrentSample(const double dCurrentSample, const double dTotalSamples);
    double process(const double dRate,
                   const double dCurrentSample,
                   const double dTotalSamples,
                   const int iBufferSize);
    void setTargetBeatDistance(double beatDistance);
    void setInstantaneousBpm(double instantaneousBpm);

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
    double getBeatDistance(double dThisPosition) const;
    bool syncTempo();
    bool syncPhase();

    // ControlObjects that come from EngineBuffer
    ControlObjectSlave* m_pPlayButton;
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

    // The current effective BPM of the engine
    ControlObject* m_pEngineBpm;

    // Used for bpm tapping from GUI and MIDI
    ControlPushButton* m_pButtonTap;

    // Button for sync'ing with the other EngineBuffer
    ControlPushButton* m_pButtonSync;
    ControlPushButton* m_pButtonSyncPhase;
    ControlPushButton* m_pButtonSyncTempo;

    // Button that translates the beats so the nearest beat is on the current
    // playposition.
    ControlPushButton* m_pTranslateBeats;

    double m_dLoopSize; // Only used to see if we shouldn't quantize position.
    double m_dPreviousSample;

    // Master Sync objects and values.
    ControlObject* m_pSyncMode;
    ControlObjectSlave* m_pThisBeatDistance;
    double m_dSyncTargetBeatDistance;
    double m_dSyncInstantaneousBpm;
    double m_dSyncAdjustment;
    double m_dUserOffset;

    TapFilter m_tapFilter;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;

    QString m_sGroup;
};


#endif // BPMCONTROL_H
