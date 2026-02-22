#pragma once

#include <QMap>
#include <memory>
#include <vector>

#include "audio/types.h"
#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/types.h"

// The ParametricEQEffect models the mid bands from a SSL Black EQ (242)
// with a gentle parameter range, as requested here:
// https://mixxx.discourse.group/t/sending-output-via-a-software-equalizer/16718/10#p33312
// The main use case is to tweak the room or recording sound, which is hard to achieve
// with the sharp and wide curves of the mixing EQs.

class ParametricEQEffectGroupState final : public EffectState {
  public:
    ParametricEQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~ParametricEQEffectGroupState() override = default;

    void setFilters(mixxx::audio::SampleRate sampleRate);

    // These containers are only appended in the constructor which is called on
    // the main thread, so there is no risk of allocation in the audio thread.
    std::vector<std::unique_ptr<EngineFilterBiquad1Peaking>> m_bands;
    QList<double> m_oldGain;
    QList<double> m_oldCenter;
    QList<double> m_oldQ;

    mixxx::audio::SampleRate m_oldSampleRate;

    QList<CSAMPLE*> m_pBufs;
};

class ParametricEQEffect : public EffectProcessorImpl<ParametricEQEffectGroupState> {
  public:
    ParametricEQEffect() = default;
    ~ParametricEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ParametricEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

  private:
    QString debugString() const {
        return getId();
    }

    // These QLists are only appended on the main thread in loadEngineEffectParameters,
    // so there is no risk of allocation in the audio thread.
    QList<EngineEffectParameterPointer> m_pPotGain;
    QList<EngineEffectParameterPointer> m_pPotQ;
    QList<EngineEffectParameterPointer> m_pPotCenter;

    mixxx::audio::SampleRate m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(ParametricEQEffect);
};
