#include "effects/native/butterworth8eqeffect.h"
#include "util/math.h"

// static
QString Butterworth8EQEffect::getId() {
    return "org.mixxx.effects.butterwortheq";
}

// static
EffectManifest Butterworth8EQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Butterworth8 EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Butterworth 8th order filter equalizer (flat responds, roll-off -48 db/Oct)"
        "To adjust frequency shelves see the Equalizer preferences."));

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(4.0);

    EffectManifestParameter* killLow = manifest.addButtonParameter();
    killLow->setId("killLow");
    killLow->setName(QObject::tr("Kill Low"));
    killLow->setDescription(QObject::tr("Kill the Low Filter"));
    killLow->setControlHint(EffectManifestParameter::CONTROL_TOGGLE);
    killLow->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
    killLow->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killLow->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killLow->setDefault(0);
    killLow->setMinimum(0);
    killLow->setMaximum(1);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("mid");
    mid->setName(QObject::tr("Mid"));
    mid->setDescription(QObject::tr("Gain for Band Filter"));
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setDefault(1.0);
    mid->setMinimum(0);
    mid->setMaximum(4.0);

    EffectManifestParameter* killMid = manifest.addButtonParameter();
    killMid->setId("killMid");
    killMid->setName(QObject::tr("Kill Mid"));
    killMid->setDescription(QObject::tr("Kill the Mid Filter"));
    killMid->setControlHint(EffectManifestParameter::CONTROL_TOGGLE);
    killMid->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
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
    high->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

    EffectManifestParameter* killHigh = manifest.addButtonParameter();
    killHigh->setId("killHigh");
    killHigh->setName(QObject::tr("Kill High"));
    killHigh->setDescription(QObject::tr("Kill the High Filter"));
    killHigh->setControlHint(EffectManifestParameter::CONTROL_TOGGLE);
    killHigh->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
    killHigh->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killHigh->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killHigh->setDefault(0);
    killHigh->setMinimum(0);
    killHigh->setMaximum(1);

    return manifest;
}

ButterworthEQEffectGroupState::ButterworthEQEffectGroupState()
        : low(NULL), band(NULL), high(NULL), old_low(1.0),
          old_mid(1.0), old_high(1.0) {
    m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBandBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize filters with the default values
    // TODO(rryan): use the real samplerate
    low = new EngineFilterButterworth8Low(44100, 246);
    band = new EngineFilterButterworth8Band(44100, 246, 2484);
    high = new EngineFilterButterworth8High(44100, 2484);
}

ButterworthEQEffectGroupState::~ButterworthEQEffectGroupState() {
    delete low;
    delete band;
    delete high;
    SampleUtil::free(m_pLowBuf);
    SampleUtil::free(m_pBandBuf);
    SampleUtil::free(m_pHighBuf);
}

void ButterworthEQEffectGroupState::setFilters(int sampleRate, int lowFreq,
                                               int highFreq) {
    low->setFrequencyCorners(sampleRate, lowFreq);
    band->setFrequencyCorners(sampleRate, lowFreq, highFreq);
    high->setFrequencyCorners(sampleRate, highFreq);
}

Butterworth8EQEffect::Butterworth8EQEffect(EngineEffect* pEffect,
                                         const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_pKillLow(pEffect->getButtonParameterById("killLow")),
          m_pKillMid(pEffect->getButtonParameterById("killMid")),
          m_pKillHigh(pEffect->getButtonParameterById("killHigh")),
          m_oldSampleRate(0), m_loFreq(0), m_hiFreq(0) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = new ControlObjectSlave("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlObjectSlave("[Mixer Profile]", "HiEQFrequency");
}

Butterworth8EQEffect::~Butterworth8EQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void Butterworth8EQEffect::processGroup(const QString& group,
                                       ButterworthEQEffectGroupState* pState,
                                       const CSAMPLE* pInput, CSAMPLE* pOutput,
                                       const unsigned int numSamples,
                                       const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    if (m_pKillLow->value().toInt() == 0) {
        fLow = m_pPotLow->value().toDouble();
    }
    if (m_pKillMid->value().toInt() == 0) {
        fMid = m_pPotMid->value().toDouble();
    }
    if (m_pKillHigh->value().toInt() == 0) {
        fHigh = m_pPotHigh->value().toDouble();
    }

    int sampleRate = getSampleRate();
    if (m_oldSampleRate != sampleRate ||
            (m_loFreq != static_cast<int>(m_pLoFreqCorner->get())) ||
            (m_hiFreq != static_cast<int>(m_pHiFreqCorner->get()))) {
        m_loFreq = static_cast<int>(m_pLoFreqCorner->get());
        m_hiFreq = static_cast<int>(m_pHiFreqCorner->get());
        m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate, m_loFreq, m_hiFreq);
    }

    // Process the new EQ'd signals.
    // They use up to 16 frames history so in case we are just starting,
    // 16 frames are junk, this is handled by ramp_delay
    int ramp_delay = 0;
    if (fLow || pState->old_low) {
        pState->low->process(pInput, pState->m_pLowBuf, numSamples);
        if(pState->old_low == 0) {
            ramp_delay = 30;
        }
    }
    if (fMid || pState->old_mid) {
        pState->band->process(pInput, pState->m_pBandBuf, numSamples);
        if(pState->old_mid== 0) {
            ramp_delay = 30;
        }
    }
    if (fHigh || pState->old_high) {
        pState->high->process(pInput, pState->m_pHighBuf, numSamples);
        if(pState->old_high == 0) {
            ramp_delay = 30;
        }
    }

    if (ramp_delay) {
        // first use old gains
        SampleUtil::copy3WithGain(pOutput,
                pState->m_pLowBuf, pState->old_low,
                pState->m_pBandBuf, pState->old_mid,
                pState->m_pHighBuf, pState->old_high,
                ramp_delay);
        // Now ramp the remaining frames
        SampleUtil::copy3WithRampingGain(&pOutput[ramp_delay],
                &pState->m_pLowBuf[ramp_delay], pState->old_low, fLow,
                &pState->m_pBandBuf[ramp_delay], pState->old_mid, fMid,
                &pState->m_pHighBuf[ramp_delay], pState->old_high, fHigh,
                numSamples - ramp_delay);
    } else if (fLow != pState->old_low ||
            fMid != pState->old_mid ||
            fHigh != pState->old_high) {
        SampleUtil::copy3WithRampingGain(pOutput,
                pState->m_pLowBuf, pState->old_low, fLow,
                pState->m_pBandBuf, pState->old_mid, fMid,
                pState->m_pHighBuf, pState->old_high, fHigh,
                numSamples);
    } else {
        SampleUtil::copy3WithGain(pOutput,
                pState->m_pLowBuf, fLow,
                pState->m_pBandBuf, fMid,
                pState->m_pHighBuf, fHigh,
                numSamples);
    }

    pState->old_low = fLow;
    pState->old_mid = fMid;
    pState->old_high = fHigh;
}
