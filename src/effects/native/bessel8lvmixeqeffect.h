#ifndef BESSEL8LVMIXEQEFFECT_H
#define BESSEL8LVMIXEQEFFECT_H

#include "lvmixeqbase.h"

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbessel8.h"
#include "engine/enginefilterdelay.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"


class Bessel8LVMixEQEffectGroupState :
        public LVMixEQEffectGroupState<EngineFilterBessel8Low> {
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
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    EngineEffectParameter* m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    EngineEffectParameter* m_pKillLow;
    EngineEffectParameter* m_pKillMid;
    EngineEffectParameter* m_pKillHigh;

    ControlObjectSlave* m_pLoFreqCorner;
    ControlObjectSlave* m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(Bessel8LVMixEQEffect);
};

#endif /* BESSEL8LVMIXEQEFFECT_H */
