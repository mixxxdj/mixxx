#include "effects/native/eqeffect.h"
#include "util/math.h"

// static
QString EqEffect::getId() {
    return "org.mixxx.effects.eqeffect";
}

// static
EffectManifest EqEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Filter Low"));
    low->setDescription("TODO");
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setDefault(1.);
    low->setMinimum(0);
    low->setMaximum(4.);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("mid");
    mid->setName(QObject::tr("Filter Mid"));
    mid->setDescription("TODO");
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setDefault(1.);
    mid->setMinimum(0);
    mid->setMaximum(4.);

    EffectManifestParameter* high = manifest.addParameter();
    high->setId("high");
    high->setName(QObject::tr("Filter High"));
    high->setDescription("TODO");
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(1.);
    high->setMinimum(0);
    high->setMaximum(4.);

    return manifest;
}

EqEffectGroupState::EqEffectGroupState()
        : low(NULL), band(NULL), high(NULL),old_low(1.0),
          old_mid(1.0), old_high(1.0), old_dry(0) {
    m_pLowBuf = new CSAMPLE[MAX_BUFFER_LEN];
    m_pBandBuf = new CSAMPLE[MAX_BUFFER_LEN];
    m_pHighBuf = new CSAMPLE[MAX_BUFFER_LEN];

    // Initialize filters with the default values
    low = new EngineFilterButterworth8Low(44100, 246);
    band = new EngineFilterButterworth8Band(44100, 246, 2484);
    high = new EngineFilterButterworth8High(44100, 2484);
}

EqEffectGroupState::~EqEffectGroupState() {
    delete low;
    delete band;
    delete high;
    delete m_pLowBuf;
    delete m_pBandBuf;
    delete m_pHighBuf;
}

void EqEffectGroupState::setFilters(int sampleRate, int lowFreq, int highFreq) {
    low->setFrequencyCorners(sampleRate, lowFreq);
    band->setFrequencyCorners(sampleRate, lowFreq, highFreq);
    high->setFrequencyCorners(sampleRate, highFreq);
}

EqEffect::EqEffect(EngineEffect* pEffect,
                   const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_oldSampleRate(0), m_loFreq(0), m_hiFreq(0) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = new ControlObjectSlave("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlObjectSlave("[Mixer Profile]", "HiEQFrequency");
}

EqEffect::~EqEffect() {
    //qDebug() << debugString() << "destroyed";
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void EqEffect::processGroup(const QString& group,
        EqEffectGroupState* pState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const unsigned int numSamples,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    fLow = m_pPotLow->value().toDouble();
    fMid = m_pPotMid->value().toDouble();
    fHigh = m_pPotHigh->value().toDouble();

    // tweak gains for RGBW
    float fDry = qMin(qMin(fLow, fMid), fHigh);
    fLow -= fDry;
    fMid -= fDry;
    fHigh -= fDry;

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
        SampleUtil::copy4WithGain(pOutput,
                pInput, pState->old_dry,
                pState->m_pLowBuf, pState->old_low,
                pState->m_pBandBuf, pState->old_mid,
                pState->m_pHighBuf, pState->old_high,
                ramp_delay);
        // Now ramp the remaining frames
        SampleUtil::copy4WithRampingGain(&pOutput[ramp_delay],
                &pInput[ramp_delay], pState->old_dry, fDry,
                &pState->m_pLowBuf[ramp_delay], pState->old_low, fLow,
                &pState->m_pBandBuf[ramp_delay], pState->old_mid, fMid,
                &pState->m_pHighBuf[ramp_delay], pState->old_high, fHigh,
                numSamples - ramp_delay);
    } else if (fLow != pState->old_low ||
            fMid != pState->old_mid ||
            fHigh != pState->old_high ||
            fDry != pState->old_dry) {
        SampleUtil::copy4WithRampingGain(pOutput,
                pInput, pState->old_dry, fDry,
                pState->m_pLowBuf, pState->old_low, fLow,
                pState->m_pBandBuf, pState->old_mid, fMid,
                pState->m_pHighBuf, pState->old_high, fHigh,
                numSamples);
    } else {
        SampleUtil::copy4WithGain(pOutput,
                pInput, fDry,
                pState->m_pLowBuf, fLow,
                pState->m_pBandBuf, fMid,
                pState->m_pHighBuf, fHigh,
                numSamples);
    }

    pState->old_low = fLow;
    pState->old_mid = fMid;
    pState->old_high = fHigh;
    pState->old_dry = fDry;
}
