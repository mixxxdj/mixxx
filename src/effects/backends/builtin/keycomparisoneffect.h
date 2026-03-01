#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

/// Tracks how many audio frames have elapsed since the last piano note
/// was triggered, used to schedule the next one.
class KeyComparisonGroupState final : public EffectState {
  public:
    explicit KeyComparisonGroupState(
            const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
    }
    ~KeyComparisonGroupState() override = default;

    std::size_t framesSinceLastNote = 0;
};

/// Plays a pitched piano note at a configurable beat interval so the DJ
/// can match it by ear to identify the musical key of a track.
class KeyComparisonEffect
        : public EffectProcessorImpl<KeyComparisonGroupState> {
  public:
    KeyComparisonEffect() = default;
    ~KeyComparisonEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>&
                    parameters) override;

    void processChannel(
            KeyComparisonGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    EngineEffectParameterPointer m_pKeyParameter;
    EngineEffectParameterPointer m_pTuningParameter;
    EngineEffectParameterPointer m_pBpmParameter;
    EngineEffectParameterPointer m_pMeasureParameter;
    EngineEffectParameterPointer m_pSyncParameter;
    EngineEffectParameterPointer m_pGainParameter;

    DISALLOW_COPY_AND_ASSIGN(KeyComparisonEffect);
};
