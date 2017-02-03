#include <effects/native/biquadfullkilleqeffect.h>
#include "util/math.h"

namespace {

static const double kMinimumFrequency = 10.0;
static const double kMaximumFrequency = kStartupSamplerate / 2;
static const double kStartupMidFreq = 1100.0;
static const double kQBoost = 0.3;
static const double kQKill = 0.9;
static const double kQLowKillShelve = 0.4;
static const double kQHighKillShelve = 0.4;
static const double kKillGain = -23;
static const double kBesselStartRatio = 0.25;


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
    if (value > kBesselStartRatio) {
        return ratio2db(value);
    }
    double startDB = ratio2db(kBesselStartRatio);
    value = 1 - (value / kBesselStartRatio);
    return (kKillGain - startDB) * value + startDB;

}

double knobValueToBesselRatio (double value, bool kill) {
    if (kill) {
        return 0.0;
    }
    return math_min(value / kBesselStartRatio, 1.0);
}

} // anonymous namesspace


// static
QString BiquadFullKillEQEffect::getId() {
    return "org.mixxx.effects.dbiquadquadeq";
}

// static
EffectManifest BiquadFullKillEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Biquad Full Kill Equalizer"));
    manifest.setShortName(QObject::tr("BQ EQ/ISO"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A 3-band Equalizer that combines an Equalizer and an Isolator circuit to offer gentle slopes and full kill.") +
        " " +  QObject::tr(
        "To adjust frequency shelves see the Equalizer preferences."));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMixingEQ(true);

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(4.0);

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
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setNeutralPointOnScale(0.5);
    mid->setDefault(1.0);
    mid->setMinimum(0);
    mid->setMaximum(4.0);

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
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setNeutralPointOnScale(0.5);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

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

BiquadFullKillEQEffectGroupState::BiquadFullKillEQEffectGroupState()
        : m_oldLowBoost(0),
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
          m_rampHoldOff(kRampDone),
          m_groupDelay(0),
          m_oldSampleRate(kStartupSamplerate) {

    m_pLowBuf = std::make_unique<SampleBuffer>(MAX_BUFFER_LEN);
    m_pBandBuf = std::make_unique<SampleBuffer>(MAX_BUFFER_LEN);
    m_pHighBuf = std::make_unique<SampleBuffer>(MAX_BUFFER_LEN);
    m_pTempBuf = std::make_unique<SampleBuffer>(MAX_BUFFER_LEN);

    // Initialize the filters with default parameters

    m_lowBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, kStartupLoFreq, kQBoost);
    m_midBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, kStartupMidFreq, kQBoost);
    m_highBoost = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, kStartupHiFreq, kQBoost);
    m_lowKill = std::make_unique<EngineFilterBiquad1LowShelving>(
            kStartupSamplerate, kStartupLoFreq * 2, kQLowKillShelve);
    m_midKill = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate, kStartupMidFreq, kQKill);
    m_highKill = std::make_unique<EngineFilterBiquad1HighShelving>(
            kStartupSamplerate, kStartupHiFreq / 2, kQHighKillShelve);
    m_lvMixIso = std::make_unique<LVMixEQEffectGroupState<EngineFilterBessel4Low>>();

    setFilters(kStartupSamplerate, kStartupLoFreq, kStartupHiFreq);
}

BiquadFullKillEQEffectGroupState::~BiquadFullKillEQEffectGroupState() {
}

void BiquadFullKillEQEffectGroupState::setFilters(
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
    m_lowKill->setFrequencyCorners(
            sampleRate, lowCenter * 2, kQLowKillShelve, m_oldLowKill);
    m_midKill->setFrequencyCorners(
            sampleRate, midCenter, kQKill, m_oldMidKill);
    m_highKill->setFrequencyCorners(
            sampleRate, highCenter / 2, kQHighKillShelve, m_oldHighKill);

    m_lvMixIso->setFilters(sampleRate, lowFreqCorner, highFreqCorner);
}

BiquadFullKillEQEffect::BiquadFullKillEQEffect(EngineEffect* pEffect,
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

BiquadFullKillEQEffect::~BiquadFullKillEQEffect() {
}

void BiquadFullKillEQEffect::processChannel(
        const ChannelHandle& handle,
        BiquadFullKillEQEffectGroupState* pState,
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

    if (bqGainLow != 0.0) {
        ++activeFilters;
    }
    if (bqGainMid != 0.0) {
        ++activeFilters;
    }
    if (bqGainHigh != 0.0) {
        ++activeFilters;
    }

    QVarLengthArray<const CSAMPLE*, 3> inBuffer;
    QVarLengthArray<CSAMPLE*, 3> outBuffer;

    if (activeFilters == 2) {
        inBuffer.append(pInput);
        outBuffer.append(pState->m_pTempBuf->data());
        inBuffer.append(pState->m_pTempBuf->data());
        outBuffer.append(pOutput);
    }
    else
    {
        inBuffer.append(pInput);
        outBuffer.append(pOutput);
        inBuffer.append(pOutput);
        outBuffer.append(pState->m_pTempBuf->data());
        inBuffer.append(pState->m_pTempBuf->data());
        outBuffer.append(pOutput);
    }

    int bufIndex = 0;

    if (bqGainLow > 0.0) {
        if (bqGainLow != pState->m_oldLowBoost) {
            double lowCenter = getCenterFrequency(
                    kMinimumFrequency, pState->m_loFreqCorner);
            pState->m_lowBoost->setFrequencyCorners(
                    sampleRate, lowCenter, kQBoost, bqGainLow);
            pState->m_oldLowBoost = bqGainLow;
        }
        pState->m_lowBoost->process(
                inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        pState->m_lowKill->pauseFilter();
        ++bufIndex;
    } else if (bqGainLow < 0.0) {
        if (bqGainLow != pState->m_oldLowKill) {
            double lowCenter = getCenterFrequency(
                    kMinimumFrequency, pState->m_loFreqCorner);
            pState->m_lowKill->setFrequencyCorners(
                    sampleRate, lowCenter * 2, kQLowKillShelve, bqGainLow);
            pState->m_oldLowKill = bqGainLow;
        }
        pState->m_lowBoost->pauseFilter();
        pState->m_lowKill->process(
                inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_lowBoost->pauseFilter();
        pState->m_lowKill->pauseFilter();
    }

    if (bqGainMid > 0.0) {
        if (bqGainMid != pState->m_oldMidBoost) {
            double midCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_highFreqCorner);
            pState->m_midBoost->setFrequencyCorners(
                    sampleRate, midCenter, kQBoost, bqGainMid);
            pState->m_oldMidBoost = bqGainMid;
        }
        pState->m_midBoost->process(
                inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        pState->m_midKill->pauseFilter();
        ++bufIndex;
    } else if (bqGainMid < 0.0) {
        if (bqGainMid != pState->m_oldMidKill) {
            double midCenter = getCenterFrequency(
                    pState->m_loFreqCorner, pState->m_highFreqCorner);
            pState->m_midKill->setFrequencyCorners(
                    sampleRate, midCenter, kQKill, bqGainMid);
            pState->m_oldMidKill = bqGainMid;
        }
        pState->m_midBoost->pauseFilter();
        pState->m_midKill->process(
                inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_midBoost->pauseFilter();
        pState->m_midKill->pauseFilter();
    }

    if (bqGainHigh > 0.0) {
        if (bqGainHigh != pState->m_oldHighBoost) {
            double highCenter = getCenterFrequency(
                    pState->m_highFreqCorner, kMaximumFrequency);
            pState->m_highBoost->setFrequencyCorners(
                    sampleRate, highCenter, kQBoost, bqGainHigh);
            pState->m_oldHighBoost = bqGainHigh;
        }
        pState->m_highBoost->process(
                inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        pState->m_highKill->pauseFilter();
        ++bufIndex;
    } else if (bqGainHigh < 0.0) {
        if (bqGainHigh != pState->m_oldHighKill) {
            double highCenter = getCenterFrequency(
                    pState->m_highFreqCorner, kMaximumFrequency);
            pState->m_highKill->setFrequencyCorners(
                    sampleRate, highCenter / 2, kQHighKillShelve, bqGainHigh);
            pState->m_oldHighKill = bqGainHigh;
        }
        pState->m_highBoost->pauseFilter();
        pState->m_highKill->process(
                inBuffer[bufIndex], outBuffer[bufIndex], numSamples);
        ++bufIndex;
    } else {
        pState->m_highBoost->pauseFilter();
        pState->m_highKill->pauseFilter();
    }

    if (activeFilters == 0) {
        SampleUtil::copy(pOutput, pInput, numSamples);
    }

    if (enableState == EffectProcessor::DISABLING) {
        pState->m_lowBoost->pauseFilter();
        pState->m_midBoost->pauseFilter();
        pState->m_highBoost->pauseFilter();
        pState->m_lowKill->pauseFilter();
        pState->m_midKill->pauseFilter();
        pState->m_highKill->pauseFilter();
    }

    if (enableState == EffectProcessor::DISABLING) {
        pState->m_lvMixIso->processChannelAndPause(pOutput, pOutput, numSamples);
    } else {
        double fLow = knobValueToBesselRatio(
                m_pPotLow->value(), m_pKillLow->toBool());
        double fMid = knobValueToBesselRatio(
                m_pPotMid->value(), m_pKillMid->toBool());
        double fHigh = knobValueToBesselRatio(
                m_pPotHigh->value(), m_pKillHigh->toBool());
        pState->m_lvMixIso->processChannel(
                pInput, pOutput, numSamples, sampleRate, fLow, fMid, fHigh,
                m_pLoFreqCorner->get(), m_pHiFreqCorner->get());
    }
}

