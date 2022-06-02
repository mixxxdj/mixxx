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
    PitchShiftGroupState(const mixxx::EngineParameters& engineParameters);

    ~PitchShiftGroupState() override;
    void initializeBuffer(const mixxx::EngineParameters& engineParameters);
    void audioParametersChanged(const mixxx::EngineParameters& engineParameters);

    std::unique_ptr<RubberBand::RubberBandStretcher> m_pRubberBand;
    CSAMPLE* m_retrieveBuffer[2];
};

class PitchShiftEffect final : public EffectProcessorImpl<PitchShiftGroupState> {
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
