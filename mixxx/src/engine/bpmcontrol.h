// bpmcontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include "engine/enginecontrol.h"

class ControlBeat;
class ControlObject;
class ControlPushButton;
class EngineBuffer;

class BpmControl : public EngineControl {
    Q_OBJECT

    public:
    BpmControl(const char* _group, const ConfigObject<ConfigValue>* _config);
    ~BpmControl();
    
    void setOtherEngineBuffer(EngineBuffer* pOtherEngineBuffer) {
        m_pOtherEngineBuffer = pOtherEngineBuffer;
    }

    double getBpm();
    
public slots:
    
    void slotSetEngineBpm(double);
    void slotFileBpmChanged(double);
    void slotControlBeatSync(double);
    
    
private:
    
    ControlObject* m_pRateSlider;
    ControlObject* m_pRateRange;
    ControlObject* m_pRateDir;
    
    /** The current loaded file's detected BPM */
    ControlObject* m_pFileBpm;
    
    /** Control used to input desired playback BPM */
    ControlBeat* m_pEngineBpm;
    
    /** Button for sync'ing with the other EngineBuffer */
    ControlPushButton* m_pButtonSync;
    
    EngineBuffer* m_pOtherEngineBuffer;

};


#endif /* BPMCONTROL_H */
