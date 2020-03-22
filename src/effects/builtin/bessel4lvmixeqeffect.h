#ifndef BESSEL4LVMIXEQEFFECT_H
#define BESSEL4LVMIXEQEFFECT_H

#include <QMap>

#include "control/controlproxy.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "effects/builtin/lvmixeqbase.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbessel4.h"
#include "engine/filters/enginefilterdelay.h"
#include "util/class.h"
#include "util/types.h"
#include "util/defs.h"

class Bessel4LVMixEQEffectGroupState :
        public LVMixEQEffectGroupState<EngineFilterBessel4Low> {
  public:
      Bessel4LVMixEQEffectGroupState(const mixxx::EngineParameters& bufferParameters)
          : LVMixEQEffectGroupState<EngineFilterBessel4Low>(bufferParameters) {
      }
};

class Bessel4LVMixEQEffect : public EffectProcessorImpl<Bessel4LVMixEQEffectGroupState> {
  public:
    Bessel4LVMixEQEffect(EngineEffect* pEffect);
    virtual ~Bessel4LVMixEQEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        Bessel4LVMixEQEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
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

    ControlProxy* m_pLoFreqCorner;
    ControlProxy* m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(Bessel4LVMixEQEffect);
};

#endif /* BESSEL4LVMIXEQEFFECT_H */
