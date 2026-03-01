#pragma once

#include <QMap>
#include <vector>

#include "effects/backends/builtin/pianosample.h"
#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

/// Holds the synthesised piano sample for one deck. The sample is regenerated
/// whenever the engine sample rate changes (rare in practice) and reused for
/// every buffer callback in between, so the synthesis cost never hits the
/// audio thread during normal playback.
class KeyComparisonGroupState final : public EffectState {
  public:
    explicit KeyComparisonGroupState(
            const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
    }
    ~KeyComparisonGroupState() override = default;

    void audioParametersChanged(
            const mixxx::EngineParameters& engineParameters);

    std::vector<CSAMPLE> pianoSample;
    std::size_t framesSinceLastNote = 0;
};

/// Plays a short pitched piano tone at a configurable beat interval.
/// The DJ tunes the Key knob until the note matches the track by ear,
/// revealing the track's musical key without needing the analyser result.
///
/// The Measure knob controls how often the note fires relative to the beat
/// grid, which lets the effect work across different time signatures without
/// changing the BPM reading.
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
