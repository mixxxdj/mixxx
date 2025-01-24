#include "effects/backends/builtin/biquadfullkilleqeffect.h"

#include "effects/backends/builtin/equalizer_util.h"
#include "effects/backends/effectmanifest.h"
#include "effects/defs.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/math.h"

namespace {
constexpr mixxx::audio::SampleRate kStartupSamplerate(44100);
constexpr double kMinimumFrequency = 10.0;
constexpr double kMaximumFrequency = static_cast<double>(kStartupSamplerate) / 2.0;
constexpr double kStartupMidFreq = 1100.0;
constexpr double kQBoost = 0.3;
constexpr double kQKill = 0.9;
constexpr double kQLowKillShelve = 0.4;
constexpr double kQHighKillShelve = 0.4;
constexpr double kKillGain = -23;
constexpr double kBesselStartRatio = 0.25;

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
    if (value > kBesselStartRatio) {
        return ratio2db(value);
    }
    double startDB = ratio2db(kBesselStartRatio);
    value = 1 - (value / kBesselStartRatio);
    return (kKillGain - startDB) * value + startDB;
}

double knobValueToBesselRatio(double value, bool kill) {
    if (kill) {
        return 0.0;
    }
    return math_min(value / kBesselStartRatio, 1.0);
}

} // anonymous namespace

// static
QString BiquadFullKillEQEffect::getId() {
    return "org.mixxx.effects.biquadfullkilleq";
}

// static
EffectManifestPointer BiquadFullKillEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Biquad Full Kill Equalizer"));
    pManifest->setShortName(QObject::tr("BQ EQ/ISO"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr(
                    "A 3-band Equalizer that combines an Equalizer and an "
                    "Isolator circuit to offer gentle slopes and full kill.") +
            " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMixingEQ(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), false);
    return pManifest;
}

BiquadFullKillEQEffectGroupState::BiquadFullKillEQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_pLowBuf(engineParameters.samplesPerBuffer()),
          m_pBandBuf(engineParameters.samplesPerBuffer()),
          m_pHighBuf(engineParameters.samplesPerBuffer()),
          m_tempBuf(engineParameters.samplesPerBuffer()),
          m_oldLowBoost(0),
          m_oldMidBoost(0),
          m_oldHighBoost(0),
          m_oldLowKill(0),
          m_oldMidKill(0),
          m_oldHighKill(0),
          m_oldLow(1.0),
          m_oldMid(1.0),
          m_oldHigh(1.0),
          m_loFreqCorner(0),
          m_highFreqCorner(0),
          m_rampHoldOff(LVMixEQEffectGroupStateConstants::kRampDone),
          m_groupDelay(0),
          m_oldSampleRate(kStartupSamplerate) {
    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, LVMixEQEffectGroupStateConstants::kStartupLoFreq, kQBoost);
    m_midBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, kStartupMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, LVMixEQEffectGroupStateConstants::kStartupHiFreq, kQBoost);
    m_lowKill =
            std::make_unique<EngineFilterBiquad1LowShelving>(kStartupSamplerate,
                    LVMixEQEffectGroupStateConstants::kStartupLoFreq * 2,
                    kQLowKillShelve);
    m_midKill = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, kStartupMidFreq, kQKill);
    m_highKill = std::make_unique<EngineFilterBiquad1HighShelving>(
            kStartupSamplerate,
            LVMixEQEffectGroupStateConstants::kStartupHiFreq / 2,
            kQHighKillShelve);
    m_lvMixIso = std::make_unique<
            LVMixEQEffectGroupState<EngineFilterBessel4Low>>(engineParameters);

    setFilters(
            mixxx::audio::SampleRate(
                    static_cast<mixxx::audio::SampleRate::value_t>(kStartupSamplerate)),
            LVMixEQEffectGroupStateConstants::kStartupLoFreq,
            LVMixEQEffectGroupStateConstants::kStartupHiFreq);
}

void BiquadFullKillEQEffectGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate,
        double lowFreqCorner,
        double highFreqCorner) {
    double lowCenter = getCenterFrequency(kMinimumFrequency, lowFreqCorner);
    double midCenter = getCenterFrequency(lowFreqCorner, highFreqCorner);
    double highCenter = getCenterFrequency(highFreqCorner, kMaximumFrequency);

    m_lowBoost->setFrequencyCorners(
            sampleRate, lowCenter, kQBoost, m_oldLowBoost);
    m_midBoost->setFrequencyCorners(
            sampleRate, midCenter, kQBoost, m_oldMidBoost);
    m_highBoost->setFrequencyCorners(
            sampleRate, highCenter, kQBoost, m_oldHighBoost);
    m_lowKill->setFrequencyCorners(
            sampleRate, lowCenter * 2, kQLowKillShelve, m_oldLowKill);
    m_midKill->setFrequencyCorners(
            sampleRate, midCenter, kQKill, m_oldMidKill);
    m_highKill->setFrequencyCorners(
            sampleRate, highCenter / 2, kQHighKillShelve, m_oldHighKill);

    m_lvMixIso->setFilters(sampleRate, lowFreqCorner, highFreqCorner);
}

BiquadFullKillEQEffect::BiquadFullKillEQEffect()
        : m_pLoFreqCorner(kMixerProfile, kLowEqFrequency),
          m_pHiFreqCorner(kMixerProfile, kHighEqFrequency) {
}

void BiquadFullKillEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotMid = parameters.value("mid");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillMid = parameters.value("killMid");
    m_pKillHigh = parameters.value("killHigh");
}

// BiquadFullKillEQEffect::~BiquadFullKillEQEffect() {
// }

void BiquadFullKillEQEffect::processChannel(
        BiquadFullKillEQEffectGroupState* pState,
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
    if (bqGainLow < 0.0 || pState->m_oldLowKill < 0.0) {
        ++activeFilters;
    }
    if (bqGainMid > 0.0 || pState->m_oldMidBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainMid < 0.0 || pState->m_oldMidKill < 0.0) {
        ++activeFilters;
    }
    if (bqGainHigh > 0.0 || pState->m_oldHighBoost > 0.0) {
        ++activeFilters;
    }
    if (bqGainHigh < 0.0 || pState->m_oldHighKill < 0.0) {
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

    if (bqGainLow < 0.0 || pState->m_oldLowKill < 0.0) {
        if (bqGainLow != pState->m_oldLowKill) {
            double lowCenter = getCenterFrequency(
                    kMinimumFrequency, pState->m_loFreqCorner);
            pState->m_lowKill->setFrequencyCorners(
                    engineParameters.sampleRate(), lowCenter * 2, kQLowKillShelve, bqGainLow);
            pState->m_oldLowKill = bqGainLow;
        }
        if (bqGainLow < 0.0) {
            pState->m_lowKill->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_lowKill->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_lowKill->pauseFilter();
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

    if (bqGainMid < 0.0 || pState->m_oldMidKill < 0.0) {
        if (bqGainMid != pState->m_oldMidKill) {
            double midCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_highFreqCorner);
            pState->m_midKill->setFrequencyCorners(
                    engineParameters.sampleRate(), midCenter, kQKill, bqGainMid);
            pState->m_oldMidKill = bqGainMid;
        }
        if (bqGainMid < 0.0) {
            pState->m_midKill->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_midKill->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_midKill->pauseFilter();
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

    if (bqGainHigh < 0.0 || pState->m_oldHighKill < 0.0) {
        if (bqGainHigh != pState->m_oldHighKill) {
            double highCenter = getCenterFrequency(
                    pState->m_highFreqCorner, kMaximumFrequency);
            pState->m_highKill->setFrequencyCorners(
                    engineParameters.sampleRate(), highCenter / 2, kQHighKillShelve, bqGainHigh);
            pState->m_oldHighKill = bqGainHigh;
        }
        if (bqGainHigh < 0.0) {
            pState->m_highKill->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        } else {
            pState->m_highKill->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], engineParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_highKill->pauseFilter();
    }

    if (activeFilters == 0 && pOutput != pInput) {
        SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
    }

    if (enableState == EffectEnableState::Disabling) {
        pState->m_lowBoost->pauseFilter();
        pState->m_midBoost->pauseFilter();
        pState->m_highBoost->pauseFilter();
        pState->m_lowKill->pauseFilter();
        pState->m_midKill->pauseFilter();
        pState->m_highKill->pauseFilter();
    }

    if (enableState == EffectEnableState::Disabling) {
        pState->m_lvMixIso->processChannelAndPause(
                pOutput, pOutput, engineParameters.samplesPerBuffer());
    } else {
        double fLow = knobValueToBesselRatio(
                m_pPotLow->value(), m_pKillLow->toBool());
        double fMid = knobValueToBesselRatio(
                m_pPotMid->value(), m_pKillMid->toBool());
        double fHigh = knobValueToBesselRatio(
                m_pPotHigh->value(), m_pKillHigh->toBool());
        pState->m_lvMixIso->processChannel(pOutput,
                pOutput,
                engineParameters.samplesPerBuffer(),
                engineParameters.sampleRate(),
                fLow,
                fMid,
                fHigh,
                m_pLoFreqCorner.get(),
                m_pHiFreqCorner.get());
    }
}
