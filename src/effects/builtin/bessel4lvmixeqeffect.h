#pragma once
#include <QMap>

#include "control/controlproxy.h"
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
    Bessel4LVMixEQEffect();
    virtual ~Bessel4LVMixEQEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            Bessel4LVMixEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPotLow;
    EngineEffectParameterPointer m_pPotMid;
    EngineEffectParameterPointer m_pPotHigh;

    EngineEffectParameterPointer m_pKillLow;
    EngineEffectParameterPointer m_pKillMid;
    EngineEffectParameterPointer m_pKillHigh;

    ControlProxy* m_pLoFreqCorner;
    ControlProxy* m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(Bessel4LVMixEQEffect);
};
