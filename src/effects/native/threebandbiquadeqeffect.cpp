#include <effects/native/threebandbiquadeqeffect.h>
#include "util/math.h"

namespace {

// The defaults are tweaked to match the Xone:23 EQ
// but allow 12 dB boost instead of just 6 dB
static const int kStartupSamplerate = 44100;
static const double kMinimumFrequency = 10.0;
static const double kMaximumFrequency = kStartupSamplerate / 2;
static const double kStartupLoFreq = 50.0;
static const double kStartupMidFreq = 1100.0;
static const double kStartupHiFreq = 12000.0;
static const double kQBoost = 0.3;
static const double kQKill = 0.9;
static const double kQKillShelve = 0.4;
static const double kBoostGain = 12;
static const double kKillGain = 23;


double getCenterFrequency(double low, double high) {
    double scaleLow = log10(low);
    double scaleHigh = log10(high);

    double scaleCenter = (scaleHigh - scaleLow) / 2 + scaleLow;
    return pow(10, scaleCenter);
}

} // anonymous namesspace


// static
QString ThreeBandBiquadEQEffect::getId() {
    return "org.mixxx.effects.threebandbiquadeq";
}

// static
EffectManifest ThreeBandBiquadEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Biquad Equalizer"));
    manifest.setShortName(QObject::tr("BQ EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A 3-band Equalizer with two biquad bell filters, a shelving high pass and kill switches.") +
        " " +  QObject::tr(
        "To adjust frequency shelves see the Equalizer preferences."));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMixingEQ(true);

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(2.0);

    EffectManifestParameter* killLow = manifest.addParameter();
    killLow->setId("killLow");
    killLow->setName(QObject::tr("Kill Low"));
    killLow->setDescription(QObject::tr("Kill the Low Filter"));
    killLow->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killLow->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killLow->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killLow->setDefault(0);
    killLow->setMinimum(0);
    killLow->setMaximum(1);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("mid");
    mid->setName(QObject::tr("Mid"));
    mid->setDescription(QObject::tr("Gain for Mid Filter"));
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setNeutralPointOnScale(0.5);
    mid->setDefault(1.0);
    mid->setMinimum(0);
    mid->setMaximum(2.0);

    EffectManifestParameter* killMid = manifest.addParameter();
    killMid->setId("killMid");
    killMid->setName(QObject::tr("Kill Mid"));
    killMid->setDescription(QObject::tr("Kill the Mid Filter"));
    killMid->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killMid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killMid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killMid->setDefault(0);
    killMid->setMinimum(0);
    killMid->setMaximum(1);

    EffectManifestParameter* high = manifest.addParameter();
    high->setId("high");
    high->setName(QObject::tr("High"));
    high->setDescription(QObject::tr("Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setNeutralPointOnScale(0.5);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(2.0);

    EffectManifestParameter* killHigh = manifest.addParameter();
    killHigh->setId("killHigh");
    killHigh->setName(QObject::tr("Kill High"));
    killHigh->setDescription(QObject::tr("Kill the High Filter"));
    killHigh->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killHigh->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killHigh->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killHigh->setDefault(0);
    killHigh->setMinimum(0);
    killHigh->setMaximum(1);

    return manifest;
}

ThreeBandBiquadEQEffectGroupState::ThreeBandBiquadEQEffectGroupState()
        : m_oldLowBoost(0),
          m_oldMidBoost(0),
          m_oldHighBoost(0),
          m_oldLowCut(0),
          m_oldMidCut(0),
          m_oldHighCut(0),
          m_loFreqCorner(0),
          m_highFreqCorner(0),
          m_oldSampleRate(kStartupSamplerate) {

    m_pBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(kStartupSamplerate , kStartupLoFreq, kQBoost);
    m_midBoost = std::make_unique<EngineFilterBiquad1Peaking>(kStartupSamplerate , kStartupMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(kStartupSamplerate , kStartupHiFreq, kQBoost);
    m_lowCut = std::make_unique<EngineFilterBiquad1Peaking>(kStartupSamplerate , kStartupLoFreq, kQKill);
    m_midCut = std::make_unique<EngineFilterBiquad1Peaking>(kStartupSamplerate , kStartupMidFreq, kQKill);
    m_highCut = std::make_unique<EngineFilterBiquad1HighShelving>(kStartupSamplerate , kStartupHiFreq / 2, kQKillShelve);
}

ThreeBandBiquadEQEffectGroupState::~ThreeBandBiquadEQEffectGroupState() {
    SampleUtil::free(m_pBuf);
}

void ThreeBandBiquadEQEffectGroupState::setFilters(
        int sampleRate, double lowFreqCorner, double highFreqCorner) {

    double lowCenter = getCenterFrequency(kMinimumFrequency, lowFreqCorner);
    double midCenter = getCenterFrequency(lowFreqCorner, highFreqCorner);
    double highCenter = getCenterFrequency(highFreqCorner, kMaximumFrequency);


    m_lowBoost->setFrequencyCorners(
            sampleRate, lowCenter, kQBoost, m_oldLowBoost);
    m_midBoost->setFrequencyCorners(
            sampleRate, midCenter, kQBoost, m_oldMidBoost);
    m_highBoost->setFrequencyCorners(
            sampleRate, highCenter, kQBoost, m_oldHighBoost);
    m_lowCut->setFrequencyCorners(
            sampleRate, lowCenter, kQKill, m_oldLowCut);
    m_midCut->setFrequencyCorners(
            sampleRate, midCenter, kQKill, m_oldMidCut);
    m_highCut->setFrequencyCorners(
            sampleRate, highCenter / 2, kQKillShelve, m_oldHighCut);

}

ThreeBandBiquadEQEffect::ThreeBandBiquadEQEffect(EngineEffect* pEffect,
                                             const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_pKillLow(pEffect->getParameterById("killLow")),
          m_pKillMid(pEffect->getParameterById("killMid")),
          m_pKillHigh(pEffect->getParameterById("killHigh")) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = std::make_unique<ControlProxy>("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = std::make_unique<ControlProxy>("[Mixer Profile]", "HiEQFrequency");
}

ThreeBandBiquadEQEffect::~ThreeBandBiquadEQEffect() {
}

void ThreeBandBiquadEQEffect::processChannel(
        const ChannelHandle& handle,
        ThreeBandBiquadEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const EffectProcessor::EnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    if (pState->m_oldSampleRate != sampleRate ||
            (pState->m_loFreqCorner != m_pLoFreqCorner->get()) ||
            (pState->m_highFreqCorner != m_pHiFreqCorner->get())) {
        pState->m_loFreqCorner = m_pLoFreqCorner->get();
        pState->m_highFreqCorner = m_pHiFreqCorner->get();
        pState->m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate, pState->m_loFreqCorner, pState->m_highFreqCorner);
    }

    float fLowBoost;
    float fMidBoost;
    float fHighBoost;
    float fLowKill;
    float fMidKill;
    float fHighKill;

    if (enableState == EffectProcessor::DISABLING) {
         // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        fLowBoost = 0;
        fMidBoost = 0;
        fHighBoost = 0;
        fLowKill = 0;
        fMidKill = 0;
        fHighKill = 0;
    } else {
        float fLow = -1.f, fMid = -1.f, fHigh = -1.f;
        if (!m_pKillLow->toBool()) {
            fLow = m_pPotLow->value() -1;
        }
        if (!m_pKillMid->toBool()) {
            fMid = m_pPotMid->value() -1;
        }
        if (!m_pKillHigh->toBool()) {
            fHigh = m_pPotHigh->value() -1;
        }

        if (fLow >= 0) {
            fLowBoost = fLow * kBoostGain;
            fLowKill = 0;
        } else {
            fLowBoost = 0;
            fLowKill = fLow * kKillGain;
        }
        if (fMid >= 0) {
            fMidBoost = fMid * kBoostGain;
            fMidKill = 0;
        } else {
            fMidBoost = 0;
            fMidKill = fMid * kKillGain;
        }
        if (fHigh >= 0) {
            fHighBoost = fHigh * kBoostGain;
            fHighKill = 0;
        } else {
            fHighBoost = 0;
            fHighKill = fHigh * kKillGain;
        }
    }

    double lowCenter = getCenterFrequency(kMinimumFrequency, pState->m_loFreqCorner);
    double midCenter = getCenterFrequency(pState->m_loFreqCorner, pState->m_highFreqCorner);
    double highCenter = getCenterFrequency(pState->m_highFreqCorner, kMaximumFrequency);


    if (fLowBoost != pState->m_oldLowBoost) {
        pState->m_lowBoost->setFrequencyCorners(
                sampleRate, lowCenter, kQBoost, fLowBoost);
        pState->m_oldLowBoost = fLowBoost;
    }
    if (fMidBoost != pState->m_oldMidBoost) {
        pState->m_midBoost->setFrequencyCorners(
                sampleRate, midCenter, kQBoost, fMidBoost);
        pState->m_oldMidBoost = fMidBoost;
    }
    if (fHighBoost != pState->m_oldHighBoost) {
        pState->m_highBoost->setFrequencyCorners(
                sampleRate, highCenter, kQBoost, fHighBoost);
        pState->m_oldHighBoost = fHighBoost;
    }
    if (fLowKill != pState->m_oldLowCut) {
        pState->m_lowCut->setFrequencyCorners(
                sampleRate, lowCenter, kQKill, fLowKill);
        pState->m_oldLowCut = fLowKill;
    }
    if (fMidKill != pState->m_oldMidCut) {
        pState->m_midCut->setFrequencyCorners(
                sampleRate, midCenter, kQKill, fMidKill);
        pState->m_oldMidCut = fMidKill;
    }
    if (fHighKill != pState->m_oldHighCut) {
        pState->m_highCut->setFrequencyCorners(
                sampleRate, highCenter / 2, kQKillShelve , fHighKill);
        pState->m_oldHighCut = fHighKill;
    }

    int activeFilters = 0;

    if (fLowBoost) {
        ++activeFilters;
    }
    if (fMidBoost) {
        ++activeFilters;
    }
    if (fHighBoost) {
        ++activeFilters;
    }
    if (fLowKill) {
        ++activeFilters;
    }
    if (fMidKill) {
        ++activeFilters;
    }
    if (fHighKill) {
        ++activeFilters;
    }

    QVarLengthArray<const CSAMPLE*, 3> inBuffer;
    QVarLengthArray<CSAMPLE*, 3> outBuffer;

    if (activeFilters == 2) {
        inBuffer.append(pInput);
        outBuffer.append(pState->m_pBuf);
        inBuffer.append(pState->m_pBuf);
        outBuffer.append(pOutput);
    }
    else
    {
        inBuffer.append(pInput);
        outBuffer.append(pOutput);
        inBuffer.append(pOutput);
        outBuffer.append(pState->m_pBuf);
        inBuffer.append(pState->m_pBuf);
        outBuffer.append(pOutput);
    }

    int bufIndex = 0;
    if (fLowBoost) {
        pState->m_lowBoost->process(inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_lowBoost->pauseFilter();
    }

    if (fMidBoost) {
        pState->m_midBoost->process(inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_midBoost->pauseFilter();
    }

    if (fHighBoost) {
        pState->m_highBoost->process(inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_highBoost->pauseFilter();
    }

    if (fLowKill) {
        pState->m_lowCut->process(inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_lowCut->pauseFilter();
    }

    if (fMidKill) {
        pState->m_midCut->process(inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_midCut->pauseFilter();
    }

    if (fHighKill) {
        pState->m_highCut->process(inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_highCut->pauseFilter();
    }

    if (activeFilters == 0) {
        SampleUtil::copy(pOutput, pInput, numSamples);
    }

    if (enableState == EffectProcessor::DISABLING) {
        pState->m_lowBoost->pauseFilter();
        pState->m_midBoost->pauseFilter();
        pState->m_highBoost->pauseFilter();
        pState->m_lowCut->pauseFilter();
        pState->m_midCut->pauseFilter();
        pState->m_highCut->pauseFilter();
    }
}
