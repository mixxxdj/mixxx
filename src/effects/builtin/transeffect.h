#pragma once

#include <cstdint>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

class TransGroupState : public EffectState {
  public:
    TransGroupState(const mixxx::EngineParameters& bufferParameters)
            : EffectState(bufferParameters) {
    }

    virtual ~TransGroupState() = default;

    CSAMPLE_GAIN transFactor{0.0};
    CSAMPLE_GAIN lastSend{0.0};
    std::uint32_t playedFrames{0};
};

class TransEffect : public EffectProcessorImpl<TransGroupState> {
  public:
    TransEffect(EngineEffect* pEffect);
    virtual ~TransEffect() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(const ChannelHandle& handle,
            TransGroupState* pGroupState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPeriodParameter;
    EngineEffectParameter* m_pFadeTimeParameter;
    EngineEffectParameter* m_pCutoffTimeParameter;
    EngineEffectParameter* m_pSendParameter;
    EngineEffectParameter* m_pQuantizeParameter;

    DISALLOW_COPY_AND_ASSIGN(TransEffect);
};
