#pragma once

#include "engine/engineobject.h"
#include "util/samplebuffer.h"

class ConfigKey;
class ControlPotmeter;
class ControlProxy;

class EngineDelay : public EngineObject {
    Q_OBJECT
  public:
    EngineDelay(const ConfigKey& delayControl, bool bPersist = true);
    ~EngineDelay() override;

    void process(CSAMPLE* pInOut, const int iBufferSize) override;

    void setDelay(double newDelay);

  public slots:
    void slotDelayChanged();

  private:
    ControlPotmeter* m_pDelayPot;
    ControlProxy* m_pSampleRate;
    mixxx::SampleBuffer m_delayBuffer;
    int m_iDelayPos;
    int m_iDelay;
};
