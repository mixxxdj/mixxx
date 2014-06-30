#include "effects/native/eqdefault.h"
#include "util/math.h"

// static
QString EQDefault::getId() {
    return "org.mixxx.effects.eqdefault";
}

// static
EffectManifest EQDefault::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Default EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("The default Equalizer featuring \
3 EngineFilterButterworth which can be modified from preferences");
    manifest.setIsEQ(true);

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription("Gain for Low Filter");
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setDefault(1.);
    low->setMinimum(0);
    low->setMaximum(4.);

    EffectManifestParameter* killLow = manifest.addButtonParameter();
    killLow->setId("killLow");
    killLow->setName(QObject::tr("Kill Low"));
    killLow->setDescription("Kill for Low Filter");
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
    mid->setDescription("Gain for Band Filter");
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setDefault(1.);
    mid->setMinimum(0);
    mid->setMaximum(4.);

    EffectManifestParameter* killMid = manifest.addButtonParameter();
    killMid->setId("killMid");
    killMid->setName(QObject::tr("Kill Mid"));
    killMid->setDescription("Kill for Mid Filter");
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
    high->setDescription("Gain for High Filter");
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(1.);
    high->setMinimum(0);
    high->setMaximum(4.);

    EffectManifestParameter* killHigh = manifest.addButtonParameter();
    killHigh->setId("killHigh");
    killHigh->setName(QObject::tr("Kill High"));
    killHigh->setDescription("Kill for High Filter");
    killHigh->setControlHint(EffectManifestParameter::CONTROL_TOGGLE);
    killHigh->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
    killHigh->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killHigh->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killHigh->setDefault(0);
    killHigh->setMinimum(0);
    killHigh->setMaximum(1);

    return manifest;
}

EQDefaultGroupState::EQDefaultGroupState()
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

EQDefaultGroupState::~EQDefaultGroupState() {
    delete low;
    delete band;
    delete high;
    delete m_pLowBuf;
    delete m_pBandBuf;
    delete m_pHighBuf;
}

void EQDefaultGroupState::setFilters(int sampleRate, int lowFreq, int highFreq) {
    low->setFrequencyCorners(sampleRate, lowFreq);
    band->setFrequencyCorners(sampleRate, lowFreq, highFreq);
    high->setFrequencyCorners(sampleRate, highFreq);
}

EQDefault::EQDefault(EngineEffect* pEffect,
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

EQDefault::~EQDefault() {
    //qDebug() << debugString() << "destroyed";
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void EQDefault::processGroup(const QString& group,
        EQDefaultGroupState* pState,
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

    qDebug() << "fLow:" << fLow << " ";
    qDebug() << "fMid:" << fMid << " ";
    qDebug() << "fHigh:" << fHigh << " ";
    qDebug() << endl;
    qDebug() << "killLow" << m_pKillLow->value().toInt() << " ";
    qDebug() << "killMid" << m_pKillMid->value().toInt() << " ";
    qDebug() << "killHigh" << m_pKillHigh->value().toInt() << " ";
    qDebug() << endl;

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
