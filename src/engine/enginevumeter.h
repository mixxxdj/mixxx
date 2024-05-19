#pragma once

#include "control/controlobject.h"
#include "control/pollingcontrolproxy.h"
#include "engine/engineobject.h"

class EngineVuMeter : public EngineObject {
    Q_OBJECT
  public:
    EngineVuMeter(const QString& group, const QString& legacyGroup = QString());

    virtual void process(CSAMPLE* pInOut, const int iBufferSize);

    void reset();

  private:
    void doSmooth(CSAMPLE &currentVolume, CSAMPLE newVolume);

    ControlObject m_vuMeter;
    ControlObject m_vuMeterLeft;
    ControlObject m_vuMeterRight;
    CSAMPLE m_fRMSvolumeL;
    CSAMPLE m_fRMSvolumeSumL;
    CSAMPLE m_fRMSvolumeR;
    CSAMPLE m_fRMSvolumeSumR;
    unsigned int m_samplesCalculated;

    ControlObject m_peakIndicator;
    ControlObject m_peakIndicatorLeft;
    ControlObject m_peakIndicatorRight;
    int m_peakDurationL;
    int m_peakDurationR;

    PollingControlProxy m_sampleRate;
};
