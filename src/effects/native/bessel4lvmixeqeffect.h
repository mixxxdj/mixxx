#ifndef BESSEL4LVMIXEQEFFECT_H
#define BESSEL4LVMIXEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbessel4.h"
#include "engine/enginefilterdelay.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

const int kMaxDelay4 = 3300; // allows a 30 Hz filter at 97346;

class Bessel4LVMixEQEffectGroupState {
  public:
    Bessel4LVMixEQEffectGroupState();
    virtual ~Bessel4LVMixEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterBessel4Low* m_low1;
    EngineFilterBessel4Low* m_low2;
    EngineFilterDelay<kMaxDelay4>* m_delay2;
    EngineFilterDelay<kMaxDelay4>* m_delay3;

    double old_low;
    double old_mid;
    double old_high;

    unsigned int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class Bessel4LVMixEQEffect : public GroupEffectProcessor<Bessel4LVMixEQEffectGroupState> {
  public:
    Bessel4LVMixEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~Bessel4LVMixEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      Bessel4LVMixEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    EngineEffectParameter* m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    ControlObjectSlave* m_pLoFreqCorner;
    ControlObjectSlave* m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(Bessel4LVMixEQEffect);
};

#endif /* BESSEL4LVMIXEQEFFECT_H */
