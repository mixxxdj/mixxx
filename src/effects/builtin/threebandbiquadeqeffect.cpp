#include "effects/builtin/threebandbiquadeqeffect.h"

#include "effects/builtin/equalizer_util.h"
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
static const double kKillGain = -26;


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
    pManifest->setDescription(QObject::tr(
        "A 3-band Equalizer with two biquad bell filters, a shelving high pass and kill switches.") +
        " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMixingEQ(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), true);
    return pManifest;
}

ThreeBandBiquadEQEffectGroupState::ThreeBandBiquadEQEffectGroupState(
      const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_tempBuf(bufferParameters.samplesPerBuffer()),
          m_oldLowBoost(0),
          m_oldMidBoost(0),
          m_oldHighBoost(0),
          m_oldLowCut(0),
          m_oldMidCut(0),
          m_oldHighCut(0),
          m_loFreqCorner(0),
          m_highFreqCorner(0),
          m_oldSampleRate(bufferParameters.sampleRate()) {

    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            bufferParameters.sampleRate() , kStartupLoFreq, kQBoost);
    m_midBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            bufferParameters.sampleRate() , kStartupMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            bufferParameters.sampleRate() , kStartupHiFreq, kQBoost);
    m_lowCut = std::make_unique<EngineFilterBiquad1Peaking>(
            bufferParameters.sampleRate() , kStartupLoFreq, kQKill);
    m_midCut = std::make_unique<EngineFilterBiquad1Peaking>(
            bufferParameters.sampleRate() , kStartupMidFreq, kQKill);
    m_highCut = std::make_unique<EngineFilterBiquad1HighShelving>(
            bufferParameters.sampleRate() , kStartupHiFreq / 2, kQKillShelve);
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

ThreeBandBiquadEQEffect::ThreeBandBiquadEQEffect(EngineEffect* pEffect)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_pKillLow(pEffect->getParameterById("killLow")),
          m_pKillMid(pEffect->getParameterById("killMid")),
          m_pKillHigh(pEffect->getParameterById("killHigh")) {
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
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    if (pState->m_oldSampleRate != bufferParameters.sampleRate() ||
            (pState->m_loFreqCorner != m_pLoFreqCorner->get()) ||
            (pState->m_highFreqCorner != m_pHiFreqCorner->get())) {
        pState->m_loFreqCorner = m_pLoFreqCorner->get();
        pState->m_highFreqCorner = m_pHiFreqCorner->get();
        pState->m_oldSampleRate = bufferParameters.sampleRate();
        pState->setFilters(bufferParameters.sampleRate(), pState->m_loFreqCorner, pState->m_highFreqCorner);
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
                    bufferParameters.sampleRate(), lowCenter, kQBoost, bqGainLow);
            pState->m_oldLowBoost = bqGainLow;
        }
        if (bqGainLow > 0.0) {
            pState->m_lowBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        } else {
            pState->m_lowBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
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
                    bufferParameters.sampleRate(), lowCenter, kQKill, bqGainLow);
            pState->m_oldLowCut = bqGainLow;
        }
        if (bqGainLow < 0.0) {
            pState->m_lowCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        } else {
            pState->m_lowCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
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
                    bufferParameters.sampleRate(), midCenter, kQBoost, bqGainMid);
            pState->m_oldMidBoost = bqGainMid;
        }
        if (bqGainMid > 0.0) {
            pState->m_midBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        } else {
            pState->m_midBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
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
                    bufferParameters.sampleRate(), midCenter, kQKill, bqGainMid);
            pState->m_oldMidCut = bqGainMid;
        }
        if (bqGainMid < 0.0) {
            pState->m_midCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        } else {
            pState->m_midCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
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
                    bufferParameters.sampleRate(), highCenter, kQBoost, bqGainHigh);
            pState->m_oldHighBoost = bqGainHigh;
        }
        if (bqGainHigh > 0.0) {
            pState->m_highBoost->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        } else {
            pState->m_highBoost->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
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
                    bufferParameters.sampleRate(), highCenter / 2, kQKillShelve, bqGainHigh);
            pState->m_oldHighCut = bqGainHigh;
        }
        if (bqGainHigh < 0.0) {
            pState->m_highCut->process(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        } else {
            pState->m_highCut->processAndPauseFilter(
                    inBuffer[bufIndex], outBuffer[bufIndex], bufferParameters.samplesPerBuffer());
        }
        ++bufIndex;
    } else {
        pState->m_highCut->pauseFilter();
    }

    if (activeFilters == 0) {
        SampleUtil::copy(pOutput, pInput, bufferParameters.samplesPerBuffer());
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
