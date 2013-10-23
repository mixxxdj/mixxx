#ifndef ENGINEFILTEREFFECT_H
#define ENGINEFILTEREFFECT_H

#include "engine/engineobject.h"

class ControlObject;
class ControlPushButton;
class EngineFilterButterworth8Low;
class EngineFilterButterworth8Band;
class EngineFilterButterworth8High;

class EngineFilterEffect : public EngineObject {
  public:
    EngineFilterEffect(const char* group);
    virtual ~EngineFilterEffect();

    void process(const CSAMPLE* pIn, const CSAMPLE* pOut, const int iBufferSize);

  private:
    void applyFilters(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize,
                      double depth);

    // Buffers for old filter's value and for bandpass filter
    CSAMPLE* m_pCrossfade_buffer;
    CSAMPLE* m_pBandpass_buffer;
    EngineFilterButterworth8Low* m_pLowFilter;
    EngineFilterButterworth8Band* m_pBandpassFilter;
    EngineFilterButterworth8High* m_pHighFilter;

    ControlObject* m_pPotmeterDepth;
    ControlPushButton* m_pFilterEnable;

    double m_old_depth;
};

#endif // ENGINEFILTEREFFECT_H
