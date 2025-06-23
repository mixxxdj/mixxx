#include "effects/backends/builtin/linkwitzriley8eqeffect.h"

#include "effects/backends/builtin/equalizer_util.h"
#include "effects/backends/effectmanifest.h"
#include "effects/defs.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterlinkwitzriley8.h"
#include "util/defs.h"

namespace {
constexpr unsigned int kStartupLoFreq = 246;
constexpr unsigned int kStartupHiFreq = 2484;
} // namespace

// static
QString LinkwitzRiley8EQEffect::getId() {
    return "org.mixxx.effects.linkwitzrileyeq";
}

// static
EffectManifestPointer LinkwitzRiley8EQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("LinkwitzRiley8 Isolator"));
    pManifest->setShortName(QObject::tr("LR8 ISO"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("A Linkwitz-Riley 8th-order filter isolator (optimized "
                        "crossover, constant phase shift, roll-off -48 "
                        "dB/octave).") +
            " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setIsMixingEQ(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), false);
    return pManifest;
}

LinkwitzRiley8EQEffectGroupState::LinkwitzRiley8EQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          old_low(1.0),
          old_mid(1.0),
          old_high(1.0),
          m_oldSampleRate(engineParameters.sampleRate()),
          m_loFreq(kStartupLoFreq),
          m_hiFreq(kStartupHiFreq) {
    m_pLowBuf = SampleUtil::alloc(kMaxEngineSamples);
    m_pMidBuf = SampleUtil::alloc(kMaxEngineSamples);
    m_pHighBuf = SampleUtil::alloc(kMaxEngineSamples);

    m_low1 = new EngineFilterLinkwitzRiley8Low(engineParameters.sampleRate(), kStartupLoFreq);
    m_high1 = new EngineFilterLinkwitzRiley8High(engineParameters.sampleRate(), kStartupLoFreq);
    m_low2 = new EngineFilterLinkwitzRiley8Low(engineParameters.sampleRate(), kStartupHiFreq);
    m_high2 = new EngineFilterLinkwitzRiley8High(engineParameters.sampleRate(), kStartupHiFreq);
}

LinkwitzRiley8EQEffectGroupState::~LinkwitzRiley8EQEffectGroupState() {
    delete m_low1;
    delete m_high1;
    delete m_low2;
    delete m_high2;
    SampleUtil::free(m_pLowBuf);
    SampleUtil::free(m_pMidBuf);
    SampleUtil::free(m_pHighBuf);
}

void LinkwitzRiley8EQEffectGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate, int lowFreq, int highFreq) {
    m_low1->setFrequencyCorners(sampleRate, lowFreq);
    m_high1->setFrequencyCorners(sampleRate, lowFreq);
    m_low2->setFrequencyCorners(sampleRate, highFreq);
    m_high2->setFrequencyCorners(sampleRate, highFreq);
}

LinkwitzRiley8EQEffect::LinkwitzRiley8EQEffect()
        : m_pLoFreqCorner(kMixerProfile, kLowEqFrequency),
          m_pHiFreqCorner(kMixerProfile, kHighEqFrequency) {
}

void LinkwitzRiley8EQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotMid = parameters.value("mid");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillMid = parameters.value("killMid");
    m_pKillHigh = parameters.value("killHigh");
}

LinkwitzRiley8EQEffect::~LinkwitzRiley8EQEffect() {
}

void LinkwitzRiley8EQEffect::processChannel(
        LinkwitzRiley8EQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    if (!m_pKillLow->toBool()) {
        fLow = static_cast<float>(m_pPotLow->value());
    }
    if (!m_pKillMid->toBool()) {
        fMid = static_cast<float>(m_pPotMid->value());
    }
    if (!m_pKillHigh->toBool()) {
        fHigh = static_cast<float>(m_pPotHigh->value());
    }

    if (pState->m_oldSampleRate != engineParameters.sampleRate() ||
            (pState->m_loFreq != static_cast<int>(m_pLoFreqCorner.get())) ||
            (pState->m_hiFreq != static_cast<int>(m_pHiFreqCorner.get()))) {
        pState->m_loFreq = static_cast<int>(m_pLoFreqCorner.get());
        pState->m_hiFreq = static_cast<int>(m_pHiFreqCorner.get());
        pState->m_oldSampleRate = engineParameters.sampleRate();
        pState->setFilters(engineParameters.sampleRate(), pState->m_loFreq, pState->m_hiFreq);
    }

    pState->m_high2->process(pInput,
            pState->m_pHighBuf,
            engineParameters.samplesPerBuffer()); // HighPass first run
    pState->m_low2->process(pInput,
            pState->m_pLowBuf,
            engineParameters
                    .samplesPerBuffer()); // LowPass first run for low and bandpass

    if (fMid != pState->old_mid || fHigh != pState->old_high) {
        SampleUtil::applyRampingGain(pState->m_pHighBuf,
                static_cast<CSAMPLE_GAIN>(pState->old_high),
                fHigh,
                engineParameters.samplesPerBuffer());
        SampleUtil::addWithRampingGain(pState->m_pHighBuf,
                pState->m_pLowBuf,
                static_cast<CSAMPLE_GAIN>(pState->old_mid),
                fMid,
                engineParameters.samplesPerBuffer());
    } else {
        SampleUtil::applyGain(pState->m_pHighBuf, fHigh, engineParameters.samplesPerBuffer());
        SampleUtil::addWithGain(pState->m_pHighBuf,
                pState->m_pLowBuf,
                fMid,
                engineParameters.samplesPerBuffer());
    }

    pState->m_high1->process(pState->m_pHighBuf,
            pState->m_pMidBuf,
            engineParameters
                    .samplesPerBuffer()); // HighPass + BandPass second run
    pState->m_low1->process(pState->m_pLowBuf,
            pState->m_pLowBuf,
            engineParameters.samplesPerBuffer()); // LowPass second run

    if (fLow != pState->old_low) {
        SampleUtil::copy2WithRampingGain(pOutput,
                pState->m_pLowBuf,
                static_cast<CSAMPLE_GAIN>(pState->old_low),
                fLow,
                pState->m_pMidBuf,
                1,
                1,
                engineParameters.samplesPerBuffer());
    } else {
        SampleUtil::copy2WithGain(pOutput,
                pState->m_pLowBuf,
                fLow,
                pState->m_pMidBuf,
                1,
                engineParameters.samplesPerBuffer());
    }

    if (enableState == EffectEnableState::Disabling) {
        // we rely on the ramping to dry in EngineEffect
        // since this EQ is not fully dry at unity
        pState->m_low1->pauseFilter();
        pState->m_low2->pauseFilter();
        pState->m_high1->pauseFilter();
        pState->m_high2->pauseFilter();
        pState->old_low = 1.0;
        pState->old_mid = 1.0;
        pState->old_high = 1.0;
    } else {
        pState->old_low = fLow;
        pState->old_mid = fMid;
        pState->old_high = fHigh;
    }
}
