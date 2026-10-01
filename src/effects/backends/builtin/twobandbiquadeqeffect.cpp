#include "effects/backends/builtin/twobandbiquadeqeffect.h"

#include "effects/backends/effectmanifest.h"
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
QString TwoBandBiquadEQEffect::getId() {
    return "org.mixxx.effects.twobandbiquadeq";
}

// static
EffectManifestPointer TwoBandBiquadEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Biquad Equalizer"));
    pManifest->setShortName(QObject::tr("2BQ EQ"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("A 3-band Equalizer with two biquad bell filters, a "
                        "shelving high pass and kill switches.") +
            " " + QObject::tr("To adjust frequency shelves, go to Preferences -> Mixer."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMixingEQ(true);

    EffectManifestParameter::ValueScaler valueScaler =
            EffectManifestParameter::ValueScaler::Linear;
    ;
    double maximum = 2.0;

    EffectManifestParameterPointer low = pManifest->addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setValueScaler(valueScaler);
    low->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    low->setNeutralPointOnScale(0.5);
    low->setRange(0, 1.0, maximum);

    EffectManifestParameterPointer killLow = pManifest->addParameter();
    killLow->setId("killLow");
    killLow->setName(QObject::tr("Kill Low"));
    killLow->setDescription(QObject::tr("Kill the Low Filter"));
    killLow->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    killLow->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    killLow->setRange(0, 0, 1);

    EffectManifestParameterPointer high = pManifest->addParameter();
    high->setId("high");
    high->setName(QObject::tr("High"));
    high->setDescription(QObject::tr("Gain for High Filter"));
    high->setValueScaler(valueScaler);
    high->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    high->setNeutralPointOnScale(0.5);
    high->setRange(0, 1.0, maximum);

    EffectManifestParameterPointer killHigh = pManifest->addParameter();
    killHigh->setId("killHigh");
    killHigh->setName(QObject::tr("Kill High"));
    killHigh->setDescription(QObject::tr("Kill the High Filter"));
    killHigh->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    killHigh->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    killHigh->setRange(0, 0, 1);

    return pManifest;
}

TwoBandBiquadEQEffectGroupState::TwoBandBiquadEQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_tempBuf(engineParameters.samplesPerBuffer()),
          m_oldLowBoost(0),
          m_oldHighBoost(0),
          m_oldLowCut(0),
          m_oldHighCut(0),
          m_freqCorner(0),
          m_oldSampleRate(engineParameters.sampleRate()) {
    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLoFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupHiFreq, kQBoost);
    m_lowCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLoFreq, kQKill);
    m_highCut = std::make_unique<EngineFilterBiquad1HighShelving>(
            engineParameters.sampleRate(), kStartupHiFreq / 2, kQKillShelve);
}

void TwoBandBiquadEQEffectGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate, double freqCorner) {
    double lowCenter = getCenterFrequency(kMinimumFrequency, freqCorner);
    double highCenter = getCenterFrequency(freqCorner, kMaximumFrequency);

    m_lowBoost->setFrequencyCorners(
            sampleRate, lowCenter, kQBoost, m_oldLowBoost);
    m_highBoost->setFrequencyCorners(
            sampleRate, highCenter, kQBoost, m_oldHighBoost);
    m_lowCut->setFrequencyCorners(
            sampleRate, lowCenter, kQKill, m_oldLowCut);
    m_highCut->setFrequencyCorners(
            sampleRate, highCenter / 2, kQKillShelve, m_oldHighCut);
}

TwoBandBiquadEQEffect::TwoBandBiquadEQEffect()
        : m_pFreqCorner(kMixerProfile, kMidEqFrequency) {
}

void TwoBandBiquadEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillHigh = parameters.value("killHigh");
}

void TwoBandBiquadEQEffect::processChannel(
        TwoBandBiquadEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    if (pState->m_oldSampleRate != engineParameters.sampleRate() ||
            (pState->m_freqCorner != m_pFreqCorner.get())) {
        pState->m_freqCorner = m_pFreqCorner.get();
        pState->m_oldSampleRate = engineParameters.sampleRate();
        pState->setFilters(engineParameters.sampleRate(),
                pState->m_freqCorner);
    }

    // Ramp to dry, when disabling, this will ramp from dry when enabling as well
    double bqGainLow = 0;
    double bqGainHigh = 0;
    if (enableState != EffectEnableState::Disabling) {
        bqGainLow = knobValueToBiquadGainDb(
                m_pPotLow->value(), m_pKillLow->toBool());
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
                    kMinimumFrequency, pState->m_freqCorner);
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
                    kMinimumFrequency, pState->m_freqCorner);
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

    if (bqGainHigh > 0.0 || pState->m_oldHighBoost > 0.0) {
        if (bqGainHigh != pState->m_oldHighBoost) {
            double highCenter = getCenterFrequency(
                    pState->m_freqCorner, kMaximumFrequency);
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
                    pState->m_freqCorner, kMaximumFrequency);
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
        pState->m_highBoost->pauseFilter();
        pState->m_lowCut->pauseFilter();
        pState->m_highCut->pauseFilter();
    }
}
