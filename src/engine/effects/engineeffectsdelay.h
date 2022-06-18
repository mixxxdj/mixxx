#pragma once

#include "engine/engineobject.h"

class EngineEffectsDelay : public EngineObjectConstIn {
  public:
    EngineEffectsDelay()
            : m_delaySamples(0),
              m_oldDelaySamples(0),
              m_delayPos(0),
              m_delayBufferSize(0){};

    virtual ~EngineEffectsDelay(){};

    void setDelay(unsigned int delaySamples);

    void process(const CSAMPLE* pIn, CSAMPLE* pOutput, const int iBufferSize);

  protected:
    int m_delaySamples;
    int m_oldDelaySamples;
    int m_delayPos;
    int m_delayBufferSize;
    CSAMPLE* m_delayBuffer;
};
