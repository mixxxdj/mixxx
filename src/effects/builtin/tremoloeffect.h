#pragma once

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"


class TremoloState : public EffectState {
  public:
    TremoloState(const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters) {};
    double gain;
    unsigned int currentFrame;
    bool quantizeEnabled = false;
    bool tripletEnabled = false;
};

class TremoloEffect : public EffectProcessorImpl<TremoloState> {
  public:
    TremoloEffect(EngineEffect* pEffect);

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        TremoloState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pRateParameter;
    EngineEffectParameter* m_pWidthParameter;
    EngineEffectParameter* m_pWaveformParameter;
    EngineEffectParameter* m_pPhaseParameter;
    EngineEffectParameter* m_pQuantizeParameter;
    EngineEffectParameter* m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(TremoloEffect);
};
