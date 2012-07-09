#ifndef KEYCONTROL_H
#define KEYCONTROL_H

// bpmcontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/enginecontrol.h"
#include "tapfilter.h"

class ControlObject;
class ControlPotmeter;
//class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT

  public:
    KeyControl(const char* _group, ConfigObject<ConfigValue>* _config);
    virtual ~KeyControl();
    double getKey();
    double getRawRate();

  public slots:

    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotSetEngineKey(double);
    void slotFileKeyChanged(double);
   /*void slotControlBeatSync(double);
    void slotControlBeatSyncPhase(double);
    void slotControlBeatSyncTempo(double);
    void slotTapFilter(double,int);
    void slotBpmTap(double);*/
    void slotRateChanged(double);
    void slotUpdatedTrackKey();
   // void slotBeatsTranslate(double);

  private:
    //bool syncTempo();
    //bool syncPhase();
    double convertKey(QString);

    // ControlObjects that come from EngineBuffer
    ControlObject* m_pPlayButton;
    //ControlObject* m_pRateSlider;
    ControlPotmeter* m_pRateSlider;
    ControlObject* m_pRateRange;
    ControlObject* m_pRateDir;

    /** The current loaded file's detected BPM */
    ControlObject* m_pFileKey;

    /** The current effective BPM of the engine */
    ControlObject* m_pEngineKey;

    // Used for bpm tapping from GUI and MIDI
    //ControlPushButton* m_pButtonTap;

    /** Button for sync'ing with the other EngineBuffer */
    /*ControlPushButton* m_pButtonSync;
    ControlPushButton* m_pButtonSyncPhase;
    ControlPushButton* m_pButtonSyncTempo;*/

    // Button that translates the beats so the nearest beat is on the current
    // playposition.
    /*ControlPushButton* m_pTranslateBeats;

    TapFilter m_tapFilter;*/

    TrackPointer m_pTrack;
    double m_pKey;
};



#endif // KEYCONTROL_H
