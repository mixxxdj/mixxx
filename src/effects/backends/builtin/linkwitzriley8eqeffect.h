#pragma once

#include <QMap>

#include "control/pollingcontrolproxy.h"
#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

class EngineFilterLinkwitzRiley8High;
class EngineFilterLinkwitzRiley8Low;

class LinkwitzRiley8EQEffectGroupState : public EffectState {
  public:
    LinkwitzRiley8EQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~LinkwitzRiley8EQEffectGroupState() override;

    void setFilters(mixxx::audio::SampleRate sampleRate, int lowFreq, int highFreq);

    EngineFilterLinkwitzRiley8Low* m_low1;
    EngineFilterLinkwitzRiley8High* m_high1;
    EngineFilterLinkwitzRiley8Low* m_low2;
    EngineFilterLinkwitzRiley8High* m_high2;

    double old_low;
    double old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pMidBuf;
    CSAMPLE* m_pHighBuf;

    mixxx::audio::SampleRate m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;
};

class LinkwitzRiley8EQEffect : public EffectProcessorImpl<LinkwitzRiley8EQEffectGroupState> {
  public:
    LinkwitzRiley8EQEffect();
    ~LinkwitzRiley8EQEffect() override;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            LinkwitzRiley8EQEffectGroupState* pState,
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
    EngineEffectParameterPointer m_pPotMid;
    EngineEffectParameterPointer m_pPotHigh;

    EngineEffectParameterPointer m_pKillLow;
    EngineEffectParameterPointer m_pKillMid;
    EngineEffectParameterPointer m_pKillHigh;

    PollingControlProxy m_pLoFreqCorner;
    PollingControlProxy m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(LinkwitzRiley8EQEffect);
};
