
// bpmcontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include "engine/enginecontrol.h"
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
    double getBpm();
    double getFileBpm();

  public slots:

    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotSetEngineBpm(double);
    void slotControlBeatSync(double);
    void slotControlBeatSyncPhase(double);
    void slotControlBeatSyncTempo(double);
    void slotTapFilter(double,int);
    void slotBpmTap(double);
    void slotAdjustBpm();
    void slotUpdatedTrackBeats();
    void slotBeatsTranslate(double);

  private:
    bool syncTempo(EngineBuffer* pOtherEngineBuffer);
    bool syncPhase(EngineBuffer* pOtherEngineBuffer);

    // ControlObjects that come from EngineBuffer
    ControlObjectSlave* m_pPlayButton;
    ControlObjectSlave* m_pRateSlider;
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

    TapFilter m_tapFilter;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};


#endif // BPMCONTROL_H
