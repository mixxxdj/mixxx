#pragma once

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

struct FilterGroupState : public EffectState {
    FilterGroupState(const mixxx::EngineParameters& engineParameters);
    ~FilterGroupState();

    void setFilters(int sampleRate, double lowFreq, double highFreq);

    mixxx::SampleBuffer m_buffer;
    EngineFilterBiquad1Low* m_pLowFilter;
    EngineFilterBiquad1High* m_pHighFilter;

    double m_loFreq;
    double m_q;
    double m_hiFreq;
};

class FilterEffect : public EffectProcessorImpl<FilterGroupState> {
  public:
    FilterEffect() = default;
    ~FilterEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            FilterGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pLPF;
    EngineEffectParameterPointer m_pQ;
    EngineEffectParameterPointer m_pHPF;

    DISALLOW_COPY_AND_ASSIGN(FilterEffect);
};
