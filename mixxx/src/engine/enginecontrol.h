// EngineControl.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINECONTROL_H
#define ENGINECONTROL_H

#include <QObject>

#include "configobject.h"

class EngineControl : public QObject {
    Q_OBJECT
    public:

    EngineControl(const char * _group, const ConfigObject<ConfigValue> * _config);
    virtual ~EngineControl() = 0;
    
    virtual double process(const double currentSample, const double totalSamples) {
        return 0;
    }
    
    void setCurrentSample(const double currentSample) {
        m_dCurrentSample = currentSample;
    }
    double getCurrentSample() {
        return m_dCurrentSample;
    }

private:
    const char* getGroup() {
        return m_pGroup;
    }

    const ConfigObject<ConfigValue>* getConfig() {
        return m_pConfig;
    }
    
    const char* m_pGroup;
    const ConfigObject<ConfigValue>* m_pConfig;
    double m_dCurrentSample;
};


#endif /* ENGINECONTROL_H */
