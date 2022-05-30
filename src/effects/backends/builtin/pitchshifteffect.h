#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"

namespace RubberBand {
class RubberBandStretcher;
} // namespace RubberBand

class PitchShiftGroupState : public EffectState {
  public:
    // This is the default increment from RubberBand 1.8.1.
    static constexpr size_t kRubberBandBlockSize = 256;

    PitchShiftGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        initializeBuffer();
        audioParametersChanged(engineParameters);
    }

    virtual ~PitchShiftGroupState();

    void initializeBuffer() {
        m_retrieveBuffer[0] = SampleUtil::alloc(MAX_BUFFER_LEN);
        m_retrieveBuffer[1] = SampleUtil::alloc(MAX_BUFFER_LEN);
    }

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        m_pRubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
                engineParameters.sampleRate(),
                engineParameters.channelCount(),
                RubberBand::RubberBandStretcher::OptionProcessRealTime);

        m_pRubberBand->setMaxProcessSize(kRubberBandBlockSize);
        m_pRubberBand->setTimeRatio(1.0);
    };

    std::unique_ptr<RubberBand::RubberBandStretcher> m_pRubberBand;
    CSAMPLE* m_retrieveBuffer[2];
};

class PitchShiftEffect : public EffectProcessorImpl<PitchShiftGroupState> {
  public:
    PitchShiftEffect() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            PitchShiftGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPitchParameter;

    DISALLOW_COPY_AND_ASSIGN(PitchShiftEffect);
};
