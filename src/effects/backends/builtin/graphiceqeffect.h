#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

class EngineFilterBiquad1LowShelving;
class EngineFilterBiquad1Peaking;
class EngineFilterBiquad1HighShelving;

class GraphicEQEffectGroupState : public EffectState {
  public:
    GraphicEQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~GraphicEQEffectGroupState() override;

    void setFilters(mixxx::audio::SampleRate sampleRate);

    EngineFilterBiquad1LowShelving* m_low;
    QList<EngineFilterBiquad1Peaking*> m_bands;
    EngineFilterBiquad1HighShelving* m_high;
    QList<CSAMPLE*> m_pBufs;
    QList<double> m_oldMid;
    double m_oldLow;
    double m_oldHigh;
    float m_centerFrequencies[8];
};

class GraphicEQEffect : public EffectProcessorImpl<GraphicEQEffectGroupState> {
  public:
    GraphicEQEffect() = default;
    ~GraphicEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            GraphicEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPotLow;
    QList<EngineEffectParameterPointer> m_pPotMid;
    EngineEffectParameterPointer m_pPotHigh;

    mixxx::audio::SampleRate m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(GraphicEQEffect);
};
