#ifndef BESSEL8LVMIXEQEFFECT_H
#define BESSEL8LVMIXEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbessel8.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class Bessel8LVMixEQEffectGroupState {
  public:
    Bessel8LVMixEQEffectGroupState();
    virtual ~Bessel8LVMixEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterBessel8Low* low;
    EngineFilterBessel8Band* band;
    EngineFilterBessel8High* high;

    double old_low;
    double old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class Bessel8LVMixEQEffect : public GroupEffectProcessor<Bessel8LVMixEQEffectGroupState> {
  public:
    Bessel8LVMixEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~Bessel8LVMixEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      Bessel8LVMixEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples,
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

    int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;

    DISALLOW_COPY_AND_ASSIGN(Bessel8LVMixEQEffect);
};

#endif /* BESSEL8LVMIXEQEFFECT_H */
