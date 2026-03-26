#include "effects/backends/builtin/fourbandbiquadeqeffect.h"

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
static constexpr double kStartupLowMidFreq = 1100.0;
static constexpr double kStartupHighMidFreq = 6100.0;
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
QString FourBandBiquadEQEffect::getId() {
    return "org.mixxx.effects.fourbandbiquadeq";
}

// static
EffectManifestPointer FourBandBiquadEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Biquad Equalizer"));
    pManifest->setShortName(QObject::tr("4BQ EQ"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("A 3-band Equalizer with four biquad bell filters, a "
                        "shelving high pass and kill switches.") +
            " " + EqualizerUtil::adjustFrequencyShelvesTip());
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

    EffectManifestParameterPointer lowMid = pManifest->addParameter();
    lowMid->setId("lowMid");
    lowMid->setName(QObject::tr("lowMid"));
    lowMid->setDescription(QObject::tr("Gain for lowMid Filter"));
    lowMid->setValueScaler(valueScaler);
    lowMid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    lowMid->setNeutralPointOnScale(0.5);
    lowMid->setRange(0, 1.0, maximum);

    EffectManifestParameterPointer killLowMid = pManifest->addParameter();
    killLowMid->setId("killLowMid");
    killLowMid->setName(QObject::tr("Kill LowMid"));
    killLowMid->setDescription(QObject::tr("Kill the LowMid Filter"));
    killLowMid->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    killLowMid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    killLowMid->setRange(0, 0, 1);

    EffectManifestParameterPointer highMid = pManifest->addParameter();
    highMid->setId("highMid");
    highMid->setName(QObject::tr("HighMid"));
    highMid->setDescription(QObject::tr("Gain for HighMid Filter"));
    highMid->setValueScaler(valueScaler);
    highMid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    highMid->setNeutralPointOnScale(0.5);
    highMid->setRange(0, 1.0, maximum);

    EffectManifestParameterPointer killHighMid = pManifest->addParameter();
    killHighMid->setId("killHighMid");
    killHighMid->setName(QObject::tr("Kill HighMid"));
    killHighMid->setDescription(QObject::tr("Kill the HighMid Filter"));
    killHighMid->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    killHighMid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    killHighMid->setRange(0, 0, 1);

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

FourBandBiquadEQEffectGroupState::FourBandBiquadEQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_tempBuf(engineParameters.samplesPerBuffer()),
          m_oldLowBoost(0),
          m_oldLowMidBoost(0),
          m_oldHighMidBoost(0),
          m_oldHighBoost(0),
          m_oldLowCut(0),
          m_oldLowMidCut(0),
          m_oldHighMidCut(0),
          m_oldHighCut(0),
          m_loFreqCorner(0),
          m_midFreqCorner(0),
          m_highFreqCorner(0),
          m_oldSampleRate(engineParameters.sampleRate()) {
    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLoFreq, kQBoost);
    m_lowMidBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLowMidFreq, kQBoost);
    m_highMidBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupHighMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupHiFreq, kQBoost);
    m_lowCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLoFreq, kQKill);
    m_lowMidCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupLowMidFreq, kQBoost);
    m_highMidCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kStartupHighMidFreq, kQKill);
    m_highCut = std::make_unique<EngineFilterBiquad1HighShelving>(
            engineParameters.sampleRate(), kStartupHiFreq / 2, kQKillShelve);
}

void FourBandBiquadEQEffectGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate,
        double lowFreqCorner,
        double midFreqCorner,
        double highFreqCorner) {
    double lowCenter = getCenterFrequency(kMinimumFrequency, lowFreqCorner);
    double lowMidCenter = getCenterFrequency(lowFreqCorner, midFreqCorner);
    double highMidCenter = getCenterFrequency(midFreqCorner, highFreqCorner);
    double highCenter = getCenterFrequency(highFreqCorner, kMaximumFrequency);

    m_lowBoost->setFrequencyCorners(
            sampleRate, lowCenter, kQBoost, m_oldLowBoost);
    m_lowMidBoost->setFrequencyCorners(
            sampleRate, lowMidCenter, kQBoost, m_oldLowMidBoost);
    m_highMidBoost->setFrequencyCorners(
            sampleRate, highMidCenter, kQBoost, m_oldHighMidBoost);
    m_highBoost->setFrequencyCorners(
            sampleRate, highCenter, kQBoost, m_oldHighBoost);
    m_lowCut->setFrequencyCorners(
            sampleRate, lowCenter, kQKill, m_oldLowCut);
    m_lowMidCut->setFrequencyCorners(
            sampleRate, lowMidCenter, kQKill, m_oldLowMidCut);
    m_highMidCut->setFrequencyCorners(
            sampleRate, highMidCenter, kQKill, m_oldHighMidCut);
    m_highCut->setFrequencyCorners(
            sampleRate, highCenter / 2, kQKillShelve, m_oldHighCut);
}

FourBandBiquadEQEffect::FourBandBiquadEQEffect()
        : m_pLoFreqCorner(kMixerProfile, kLowEqFrequency),
          m_pMidFreqCorner(kMixerProfile, kMidEqFrequency),
          m_pHiFreqCorner(kMixerProfile, kHighEqFrequency) {
}

void FourBandBiquadEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotLowMid = parameters.value("lowMid");
    m_pPotHighMid = parameters.value("highMid");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillLowMid = parameters.value("killLowMid");
    m_pKillHighMid = parameters.value("killHighMid");
    m_pKillHigh = parameters.value("killHigh");
}

void FourBandBiquadEQEffect::processChannel(
        FourBandBiquadEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    if (pState->m_oldSampleRate != engineParameters.sampleRate() ||
            (pState->m_loFreqCorner != m_pLoFreqCorner.get()) ||
            (pState->m_midFreqCorner != m_pMidFreqCorner.get()) ||
            (pState->m_highFreqCorner != m_pHiFreqCorner.get())) {
        pState->m_loFreqCorner = m_pLoFreqCorner.get();
        pState->m_midFreqCorner = m_pMidFreqCorner.get();
        pState->m_highFreqCorner = m_pHiFreqCorner.get();
        pState->m_oldSampleRate = engineParameters.sampleRate();
        pState->setFilters(engineParameters.sampleRate(),
                pState->m_loFreqCorner,
                pState->m_midFreqCorner,
                pState->m_highFreqCorner);
    }

    // Ramp to dry, when disabling, this will ramp from dry when enabling as well
    double bqGainLow = 0;
    double bqGainLowMid = 0;
    double bqGainHighMid = 0;
    double bqGainHigh = 0;
    if (enableState != EffectEnableState::Disabling) {
        bqGainLow = knobValueToBiquadGainDb(
                m_pPotLow->value(), m_pKillLow->toBool());
        bqGainLowMid = knobValueToBiquadGainDb(
                m_pPotLowMid->value(), m_pKillLowMid->toBool());
        bqGainHighMid = knobValueToBiquadGainDb(
                m_pPotHighMid->value(), m_pKillHighMid->toBool());
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
    if (bqGainLowMid > 0.0 || pState->m_oldLowMidBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainLowMid < 0.0 || pState->m_oldLowMidCut < 0.0) {
        ++activeFilters;
    }
    if (bqGainHighMid > 0.0 || pState->m_oldHighMidBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainHighMid < 0.0 || pState->m_oldHighMidCut < 0.0) {
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

    if (bqGainLowMid > 0.0 || pState->m_oldLowMidBoost > 0.0) {
        if (bqGainLowMid != pState->m_oldLowMidBoost) {
            double lowMidCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_midFreqCorner);
            pState->m_lowMidBoost->setFrequencyCorners(
                    engineParameters.sampleRate(), lowMidCenter, kQBoost, bqGainLowMid);
            pState->m_oldLowMidBoost = bqGainLowMid;
        }
        if (bqGainLowMid > 0.0) {
            pState->m_lowMidBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_lowMidBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_lowMidBoost->pauseFilter();
    }

    if (bqGainLowMid < 0.0 || pState->m_oldLowMidCut < 0.0) {
        if (bqGainLowMid != pState->m_oldLowMidCut) {
            double lowMidCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_midFreqCorner);
            pState->m_lowMidCut->setFrequencyCorners(
                    engineParameters.sampleRate(), lowMidCenter, kQKill, bqGainLowMid);
            pState->m_oldLowMidCut = bqGainLowMid;
        }
        if (bqGainLowMid < 0.0) {
            pState->m_lowMidCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_lowMidCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_lowMidCut->pauseFilter();
    }

    if (bqGainHighMid > 0.0 || pState->m_oldHighMidBoost > 0.0) {
        if (bqGainHighMid != pState->m_oldHighMidBoost) {
            double highMidCenter = getCenterFrequency(
                    pState->m_midFreqCorner, pState->m_highFreqCorner);
            pState->m_highMidBoost->setFrequencyCorners(
                    engineParameters.sampleRate(), highMidCenter, kQBoost, bqGainHighMid);
            pState->m_oldHighMidBoost = bqGainHighMid;
        }
        if (bqGainHighMid > 0.0) {
            pState->m_highMidBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_highMidBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_highMidBoost->pauseFilter();
    }

    if (bqGainHighMid < 0.0 || pState->m_oldHighMidCut < 0.0) {
        if (bqGainHighMid != pState->m_oldHighMidCut) {
            double highMidCenter = getCenterFrequency(
                    pState->m_midFreqCorner, pState->m_highFreqCorner);
            pState->m_highMidCut->setFrequencyCorners(
                    engineParameters.sampleRate(), highMidCenter, kQKill, bqGainHighMid);
            pState->m_oldHighMidCut = bqGainHighMid;
        }
        if (bqGainHighMid < 0.0) {
            pState->m_highMidCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_highMidCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_highMidCut->pauseFilter();
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
        pState->m_lowMidBoost->pauseFilter();
        pState->m_highMidBoost->pauseFilter();
        pState->m_highBoost->pauseFilter();
        pState->m_lowCut->pauseFilter();
        pState->m_lowMidCut->pauseFilter();
        pState->m_highMidCut->pauseFilter();
        pState->m_highCut->pauseFilter();
    }
}
