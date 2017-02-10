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
static const double kKillGain = -23;


double getCenterFrequency(double low, double high) {
    double scaleLow = log10(low);
    double scaleHigh = log10(high);

    double scaleCenter = (scaleHigh - scaleLow) / 2 + scaleLow;
    return pow(10, scaleCenter);
}

double knobValueToBiquadGainDb (double value, bool kill) {
    if (kill) {
        return kKillGain;
    }

    double ret = value - 1;
    if (value >= 0) {
        ret *= kBoostGain;
    } else {
        ret *= -kKillGain;
    }
    return ret;
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
        : m_tempBuf(MAX_BUFFER_LEN),
          m_oldLowBoost(0),
          m_oldMidBoost(0),
          m_oldHighBoost(0),
          m_oldLowCut(0),
          m_oldMidCut(0),
          m_oldHighCut(0),
          m_loFreqCorner(0),
          m_highFreqCorner(0),
          m_oldSampleRate(kStartupSamplerate) {

    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate , kStartupLoFreq, kQBoost);
    m_midBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate , kStartupMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate , kStartupHiFreq, kQBoost);
    m_lowCut = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate , kStartupLoFreq, kQKill);
    m_midCut = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate , kStartupMidFreq, kQKill);
    m_highCut = std::make_unique<EngineFilterBiquad1HighShelving>(
            kStartupSamplerate , kStartupHiFreq / 2, kQKillShelve);
}

ThreeBandBiquadEQEffectGroupState::~ThreeBandBiquadEQEffectGroupState() {
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


    // Ramp to dry, when disabling, this will ramp from dry when enabling as well
    double bqGainLow = 0;
    double bqGainMid = 0;
    double bqGainHigh = 0;
    if (enableState != EffectProcessor::DISABLING) {
        bqGainLow = knobValueToBiquadGainDb(
                m_pPotLow->value(), m_pKillLow->toBool());
        bqGainMid = knobValueToBiquadGainDb(
                m_pPotMid->value(), m_pKillMid->toBool());
        bqGainHigh = knobValueToBiquadGainDb(
                m_pPotHigh->value(), m_pKillHigh->toBool());
    }

    int activeFilters = 0;

    if (bqGainLow > 0.0 || pState->m_oldLowBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainLow < 0.0 || pState->m_oldLowCut < 0.0) {
        ++activeFilters;
    }
    if (bqGainMid > 0.0 || pState->m_oldMidBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainMid < 0.0 || pState->m_oldMidCut < 0.0) {
        ++activeFilters;
    }
    if (bqGainHigh > 0.0 || pState->m_oldHighBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainHigh < 0.0 || pState->m_oldHighCut < 0.0) {
        ++activeFilters;
    }

    QVarLengthArray<const CSAMPLE*, 6> inBuffer;
    QVarLengthArray<CSAMPLE*, 6> outBuffer;

    if (activeFilters % 2 == 0) {
        inBuffer.append(pInput);
        outBuffer.append(pState->m_tempBuf.data());

        inBuffer.append(pState->m_tempBuf.data());
        outBuffer.append(pOutput);

        inBuffer.append(pOutput);
        outBuffer.append(pState->m_tempBuf.data());

        inBuffer.append(pState->m_tempBuf.data());
        outBuffer.append(pOutput);

        inBuffer.append(pOutput);
        outBuffer.append(pState->m_tempBuf.data());

        inBuffer.append(pState->m_tempBuf.data());
        outBuffer.append(pOutput);
    }
    else
    {
        inBuffer.append(pInput);
        outBuffer.append(pOutput);

        inBuffer.append(pOutput);
        outBuffer.append(pState->m_tempBuf.data());

        inBuffer.append(pState->m_tempBuf.data());
        outBuffer.append(pOutput);

        inBuffer.append(pOutput);
        outBuffer.append(pState->m_tempBuf.data());

        inBuffer.append(pState->m_tempBuf.data());
        outBuffer.append(pOutput);

        inBuffer.append(pOutput);
        outBuffer.append(pState->m_tempBuf.data());
    }

    int bufIndex = 0;

    if (bqGainLow > 0.0 || pState->m_oldLowBoost > 0.0) {
        if (bqGainLow != pState->m_oldLowBoost) {
            double lowCenter = getCenterFrequency(
                    kMinimumFrequency, pState->m_loFreqCorner);
            pState->m_lowBoost->setFrequencyCorners(
                    sampleRate, lowCenter, kQBoost, bqGainLow);
            pState->m_oldLowBoost = bqGainLow;
        }
        if (bqGainLow > 0.0) {
            pState->m_lowBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        } else {
            pState->m_lowBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        }
        ++bufIndex;
    } else {
        pState->m_lowBoost->pauseFilter();
    }

    if (bqGainLow < 0.0 || pState->m_oldLowCut < 0.0) {
        if (bqGainLow != pState->m_oldLowCut) {
            double lowCenter = getCenterFrequency(
                    kMinimumFrequency, pState->m_loFreqCorner);
            pState->m_lowCut->setFrequencyCorners(
                    sampleRate, lowCenter, kQKill, bqGainLow);
            pState->m_oldLowCut = bqGainLow;
        }
        if (bqGainLow < 0.0) {
            pState->m_lowCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        } else {
            pState->m_lowCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        }
        ++bufIndex;
    } else {
        pState->m_lowCut->pauseFilter();
    }

    if (bqGainMid > 0.0 || pState->m_oldMidBoost > 0.0) {
        if (bqGainMid != pState->m_oldMidBoost) {
            double midCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_highFreqCorner);
            pState->m_midBoost->setFrequencyCorners(
                    sampleRate, midCenter, kQBoost, bqGainMid);
            pState->m_oldMidBoost = bqGainMid;
        }
        if (bqGainMid > 0.0) {
            pState->m_midBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        } else {
            pState->m_midBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        }
        ++bufIndex;
    } else {
        pState->m_midBoost->pauseFilter();
    }


    if (bqGainMid < 0.0 || pState->m_oldMidCut < 0.0) {
        if (bqGainMid != pState->m_oldMidCut) {
            double midCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_highFreqCorner);
            pState->m_midCut->setFrequencyCorners(
                    sampleRate, midCenter, kQKill, bqGainMid);
            pState->m_oldMidCut = bqGainMid;
        }
        if (bqGainMid < 0.0) {
            pState->m_midCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        } else {
            pState->m_midCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        }
        ++bufIndex;
    } else {
        pState->m_midCut->pauseFilter();
    }

    if (bqGainHigh > 0.0 || pState->m_oldHighBoost > 0.0) {
        if (bqGainHigh != pState->m_oldHighBoost) {
            double highCenter = getCenterFrequency(
                    pState->m_highFreqCorner, kMaximumFrequency);
            pState->m_highBoost->setFrequencyCorners(
                    sampleRate, highCenter, kQBoost, bqGainHigh);
            pState->m_oldHighBoost = bqGainHigh;
        }
        if (bqGainHigh > 0.0) {
            pState->m_highBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        } else {
            pState->m_highBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        }
        ++bufIndex;
    } else {
        pState->m_highBoost->pauseFilter();
    }

    if (bqGainHigh < 0.0 || pState->m_oldHighCut < 0.0) {
        if (bqGainHigh != pState->m_oldHighCut) {
            double highCenter = getCenterFrequency(
                    pState->m_highFreqCorner, kMaximumFrequency);
            pState->m_highCut->setFrequencyCorners(
                    sampleRate, highCenter / 2, kQKillShelve, bqGainHigh);
            pState->m_oldHighCut = bqGainHigh;
        }
        if (bqGainHigh < 0.0) {
            pState->m_highCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        } else {
            pState->m_highCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        }
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
