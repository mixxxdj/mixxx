#pragma once

#include <QMap>
#include <vector>

#include "control/controlproxy.h"
#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/memory.h"
#include "util/sample.h"
#include "util/types.h"

// The ParametricEQEffect models the mid bands from a SSL Black EQ (242)
// with a gentle parameter range, as requested here:
// https://www.mixxx.org/forums/viewtopic.php?f=3&t=9239&p=33312&hilit=SSL#p33312
// The main use case is to tweak the room or recording sound, which is hard to achieve
// with the sharp and wide curves of the mixing EQs.

class ParametricEQEffectGroupState final : public EffectState {
  public:
    ParametricEQEffectGroupState(const mixxx::EngineParameters& bufferParameters);

    void setFilters(int sampleRate);

    std::vector<std::unique_ptr<EngineFilterBiquad1Peaking> > m_bands;
    QList<double> m_oldGain;
    QList<double> m_oldCenter;
    QList<double> m_oldQ;

    unsigned int m_oldSampleRate;

    QList<CSAMPLE*> m_pBufs;
};

class ParametricEQEffect : public EffectProcessorImpl<ParametricEQEffectGroupState> {
  public:
    ParametricEQEffect() = default;
    virtual ~ParametricEQEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ParametricEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

  private:
    QString debugString() const {
        return getId();
    }

    QList<EngineEffectParameterPointer> m_pPotGain;
    QList<EngineEffectParameterPointer> m_pPotQ;
    QList<EngineEffectParameterPointer> m_pPotCenter;

    mixxx::audio::SampleRate m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(ParametricEQEffect);
};
