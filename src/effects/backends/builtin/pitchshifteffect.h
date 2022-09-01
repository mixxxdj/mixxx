#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/circularbuffer.h"
#include "util/class.h"
#include "util/types.h"

namespace RubberBand {
class RubberBandStretcher;
} // namespace RubberBand

class PitchShiftGroupState : public EffectState {
  public:
    PitchShiftGroupState(const mixxx::EngineParameters& engineParameters);

    ~PitchShiftGroupState() override;
    void initializeBuffer(const mixxx::EngineParameters& engineParameters);
    //void audioParametersChanged(const mixxx::EngineParameters& engineParameters);

    std::unique_ptr<RubberBand::RubberBandStretcher> m_pRubberBand;
    std::unique_ptr<CircularBuffer<CSAMPLE>> m_inputBuffer;
    CSAMPLE* m_retrieveBuffer[2];
    CSAMPLE* m_offset[2];
    CSAMPLE* m_interleavedBuffer;
};

class PitchShiftEffect final : public EffectProcessorImpl<PitchShiftGroupState> {
  public:
    PitchShiftEffect();

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

    // Returns RubberBand process latency.
    SINT getGroupDelayFrames() override {
        return m_groupDelayFrames;
    }

  private:
    QString debugString() const {
        return getId();
    }

    bool m_currentFormant;
    bool m_fullRingBuffer;
    SINT m_groupDelayFrames;
    EngineEffectParameterPointer m_pPitchParameter;
    EngineEffectParameterPointer m_pRangeParameter;
    EngineEffectParameterPointer m_pSemitonesModeParameter;
    EngineEffectParameterPointer m_pFormantPreservingParameter;

    DISALLOW_COPY_AND_ASSIGN(PitchShiftEffect);
};
