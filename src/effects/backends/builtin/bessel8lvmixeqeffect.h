#pragma once

#include <QMap>

#include "control/pollingcontrolproxy.h"
#include "effects/backends/builtin/lvmixeqbase.h"
#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterbessel8.h"
#include "util/class.h"
#include "util/types.h"

class Bessel8LVMixEQEffectGroupState : public LVMixEQEffectGroupState<EngineFilterBessel8Low> {
  public:
    Bessel8LVMixEQEffectGroupState(const mixxx::EngineParameters& engineParameters)
            : LVMixEQEffectGroupState<EngineFilterBessel8Low>(engineParameters) {
    }
};

class Bessel8LVMixEQEffect : public EffectProcessorImpl<Bessel8LVMixEQEffectGroupState> {
  public:
    Bessel8LVMixEQEffect();
    ~Bessel8LVMixEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            Bessel8LVMixEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
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

    PollingControlProxy m_pLoFreqCorner;
    PollingControlProxy m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(Bessel8LVMixEQEffect);
};
