#include "effects/backends/builtin/threebandbiquadeqeffect.h"

#include "effects/backends/builtin/equalizer_util.h"
#include "effects/defs.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/math.h"

namespace {

// The defaults are tweaked to match the Xone:23 EQ
// but allow 12 dB boost instead of just 6 dB
static constexpr int kStartupSamplerate = 44100;
static constexpr double kMinimumFrequency = 10.0;
static constexpr double kMaximumFrequency = kStartupSamplerate / 2;
static constexpr double kStartupLoFreq = 50.0;
static constexpr double kStartupMidFreq = 1100.0;
static constexpr double kStartupHiFreq = 12000.0;
static constexpr double kQBoost = 0.3;
static constexpr double kQKill = 0.9;
static constexpr double kQKillShelve = 0.4;
static constexpr double kBoostGain = 12;
static constexpr double kKillGain = -26;

double getCenterFrequency(double low, double high) {
    double scaleLow = log10(low);
    double scaleHigh = log10(high);

    double scaleCenter = (scaleHigh - scaleLow) / 2 + scaleLow;
    return pow(10, scaleCenter);
}

double knobValueToBiquadGainDb(double value, bool kill) {
    if (kill) {
        return kKillGain;
    }

    double ret = value - 1;
    if (ret >= 0) {
        ret *= kBoostGain;
    } else {
        ret *= -kKillGain;
    }
    return ret;
}

} // anonymous namespace

// static
QString ThreeBandBiquadEQEffect::getId() {
    return "org.mixxx.effects.threebandbiquadeq";
}

// static
EffectManifestPointer ThreeBandBiquadEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Biquad Equalizer"));
    pManifest->setShortName(QObject::tr("BQ EQ"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("A 3-band Equalizer with two biquad bell filters, a "
                        "shelving high pass and kill switches.") +
            " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMixingEQ(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), true);
    return pManifest;
}

ThreeBandBiquadEQEffectGroupState::ThreeBandBiquadEQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_tempBuf(engineParameters.samplesPerBuffer()),
          m_oldLowBoost(0),
          m_oldMidBoost(0),
          m_oldHighBoost(0),
          m_oldLowCut(0),
          m_oldMidCut(0),
          m_oldHighCut(0),
          m_loFreqCorner(0),
          m_highFreqCorner(0),
          m_oldSampleRate(engineParameters.sampleRate()) {
    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLoFreq, kQBoost);
    m_midBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupHiFreq, kQBoost);
    m_lowCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLoFreq, kQKill);
    m_midCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupMidFreq, kQKill);
    m_highCut = std::make_unique<EngineFilterBiquad1HighShelving>(
            engineParameters.sampleRate(), kStartupHiFreq / 2, kQKillShelve);
}

void ThreeBandBiquadEQEffectGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate, double lowFreqCorner, double highFreqCorner) {
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

ThreeBandBiquadEQEffect::ThreeBandBiquadEQEffect()
        : m_pLoFreqCorner({kMixerProfile, kLowEqFrequency}),
          m_pHiFreqCorner({kMixerProfile, kHighEqFrequency}) {
}

void ThreeBandBiquadEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotMid = parameters.value("mid");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillMid = parameters.value("killMid");
    m_pKillHigh = parameters.value("killHigh");
}

void ThreeBandBiquadEQEffect::processChannel(
        ThreeBandBiquadEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    if (pState->m_oldSampleRate != engineParameters.sampleRate() ||
            (pState->m_loFreqCorner != m_pLoFreqCorner.get()) ||
            (pState->m_highFreqCorner != m_pHiFreqCorner.get())) {
        pState->m_loFreqCorner = m_pLoFreqCorner.get();
        pState->m_highFreqCorner = m_pHiFreqCorner.get();
        pState->m_oldSampleRate = engineParameters.sampleRate();
        pState->setFilters(engineParameters.sampleRate(),
                pState->m_loFreqCorner,
                pState->m_highFreqCorner);
    }

    // Ramp to dry, when disabling, this will ramp from dry when enabling as well
    double bqGainLow = 0;
    double bqGainMid = 0;
    double bqGainHigh = 0;
    if (enableState != EffectEnableState::Disabling) {
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
    } else {
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
                    engineParameters.sampleRate(), lowCenter, kQBoost, bqGainLow);
            pState->m_oldLowBoost = bqGainLow;
        }
        if (bqGainLow > 0.0) {
            pState->m_lowBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_lowBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
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
                    engineParameters.sampleRate(), lowCenter, kQKill, bqGainLow);
            pState->m_oldLowCut = bqGainLow;
        }
        if (bqGainLow < 0.0) {
            pState->m_lowCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_lowCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
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
                    engineParameters.sampleRate(), midCenter, kQBoost, bqGainMid);
            pState->m_oldMidBoost = bqGainMid;
        }
        if (bqGainMid > 0.0) {
            pState->m_midBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_midBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
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
                    engineParameters.sampleRate(), midCenter, kQKill, bqGainMid);
            pState->m_oldMidCut = bqGainMid;
        }
        if (bqGainMid < 0.0) {
            pState->m_midCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_midCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
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
                    engineParameters.sampleRate(), highCenter, kQBoost, bqGainHigh);
            pState->m_oldHighBoost = bqGainHigh;
        }
        if (bqGainHigh > 0.0) {
            pState->m_highBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_highBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
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
                    engineParameters.sampleRate(), highCenter / 2, kQKillShelve, bqGainHigh);
            pState->m_oldHighCut = bqGainHigh;
        }
        if (bqGainHigh < 0.0) {
            pState->m_highCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_highCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_highCut->pauseFilter();
    }

    if (activeFilters == 0) {
        if (pOutput != pInput) {
            SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
        }
    }

    if (enableState == EffectEnableState::Disabling) {
        pState->m_lowBoost->pauseFilter();
        pState->m_midBoost->pauseFilter();
        pState->m_highBoost->pauseFilter();
        pState->m_lowCut->pauseFilter();
        pState->m_midCut->pauseFilter();
        pState->m_highCut->pauseFilter();
    }
}
