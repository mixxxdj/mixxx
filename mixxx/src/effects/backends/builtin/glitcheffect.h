#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

namespace {
constexpr double kMaxDelay = 2.0;
}

class GlitchGroupState : public EffectState {
  public:
    GlitchGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        repeat_buf = mixxx::SampleBuffer(engineParameters.samplesPerBuffer());
    };

    void clear() {
        repeat_buf.clear();
        sample_count = 0;
    };

    mixxx::SampleBuffer repeat_buf;
    long long sample_count;
};

class GlitchEffect : public EffectProcessorImpl<GlitchGroupState> {
  public:
    GlitchEffect() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            GlitchGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pDelayParameter;
    EngineEffectParameterPointer m_pQuantizeParameter;
    EngineEffectParameterPointer m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(GlitchEffect);
};
