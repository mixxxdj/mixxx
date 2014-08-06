#include "effects/native/linearphaseeqeffect.h"
#include "util/math.h"

// static
QString LinearPhaseEQEffect::getId() {
    return "org.mixxx.effects.linearphaseeq";
}

// static
EffectManifest LinearPhaseEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("linear phase EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A trunkated Linkwitz-Riley 4th order filter equalizer with linear phase"
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

    return manifest;
}

LinearPhaseEQEffectGroupState::LinearPhaseEQEffectGroupState()
        : low(NULL),
          high(NULL),
          old_low(1.0),
          old_mid(1.0),
          old_high(1.0),
          m_windowSize(0),
          m_processPos(0) {
    m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pWindow = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pProcessBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pLastProcessBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pLastIn = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize filters with the default values
    // TODO(rryan): use the real samplerate
    low = new EngineFilterButterworth8High(44100, 246);
    high = new EngineFilterButterworth8High(44100, 2484);


}

LinearPhaseEQEffectGroupState::~LinearPhaseEQEffectGroupState() {
    delete low;
    delete high;
    SampleUtil::free(m_pLowBuf);
    SampleUtil::free(m_pHighBuf);
    SampleUtil::free(m_pWindow);
    SampleUtil::free(m_pLastIn);
    SampleUtil::free(m_pProcessBuf);
    SampleUtil::free(m_pLastProcessBuf);
}

void LinearPhaseEQEffectGroupState::setFilters(int sampleRate, int lowFreq,
                                               int highFreq) {
    low->setFrequencyCorners(sampleRate, lowFreq);
    high->setFrequencyCorners(sampleRate, highFreq);
}

LinearPhaseEQEffect::LinearPhaseEQEffect(EngineEffect* pEffect,
                                         const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_oldSampleRate(0), m_loFreq(0), m_hiFreq(0) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = new ControlObjectSlave("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlObjectSlave("[Mixer Profile]", "HiEQFrequency");
}

LinearPhaseEQEffect::~LinearPhaseEQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void LinearPhaseEQEffect::processGroup(const QString& group,
                                        LinearPhaseEQEffectGroupState* pState,
                                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                                       const unsigned int numSamples,
                                       const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    if (pState->m_windowSize != numSamples) {
        pState->m_windowSize = numSamples;
        // Create a Mono Hanning Window
        for (unsigned int i = 0; i < pState->m_windowSize; i++) {
            pState->m_pWindow[i] = 0.5 - 0.5 * cos(2 * M_PI * i / (pState->m_windowSize - 1));
        }
    }

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    fLow = m_pPotLow->value().toDouble();
    fMid = m_pPotMid->value().toDouble();
    fHigh = m_pPotHigh->value().toDouble();

    // Since this is a linear phase Filter we can subtract or add the filtered signal to the dry signal
    // The dry signal represents the low gain
    // Then the lower high pass result is added and at least the higher high pass result.
    fHigh = fHigh - fMid;
    fMid = fMid - fLow;

    int sampleRate = getSampleRate();
    if (m_oldSampleRate != sampleRate ||
            (m_loFreq != static_cast<int>(m_pLoFreqCorner->get())) ||
            (m_hiFreq != static_cast<int>(m_pHiFreqCorner->get()))) {
        m_loFreq = static_cast<int>(m_pLoFreqCorner->get());
        m_hiFreq = static_cast<int>(m_pHiFreqCorner->get());
        m_oldSampleRate = sampleRate;
        // Clamp frequency corners to the border, defined by the window size.
        // this avoids artifacts in the low band
        m_loFreq = math_max(m_loFreq, sampleRate / (int)numSamples * 5);
        m_hiFreq = math_max(m_hiFreq, m_loFreq);
        pState->setFilters(sampleRate, m_loFreq, m_hiFreq);
    }


    // Apply Hanning window to the old samples and copy them to process buffer
    for (unsigned int i = 0; i < numSamples / 2; ++i) {
        pState->m_pProcessBuf[i * 2] = pState->m_pLastIn[i * 2] * pState->m_pWindow[i];
        pState->m_pProcessBuf[i * 2 + 1] = pState->m_pLastIn[i * 2 + 1] * pState->m_pWindow[i];
    }

    // Save current samples for the next round
    // Apply Hanning window
    // copy them to process buffer
    for (unsigned int i = 0; i < numSamples / 2; ++i) {
        CSAMPLE sample = pInput[i * 2];
        pState->m_pLastIn[i * 2] = sample;
        pState->m_pProcessBuf[numSamples + i * 2] = sample * pState->m_pWindow[i + numSamples / 2];
        sample = pInput[i * 2 + 1];
        pState->m_pLastIn[i * 2 + 1] = sample;
        pState->m_pProcessBuf[numSamples + i * 2 + 1] = sample * pState->m_pWindow[i + numSamples / 2];
    }

    // delete buffer for Smear.
    for (unsigned int i = 0; i < 3000 / 2; ++i) {
        pState->m_pProcessBuf[numSamples * 2 + i] = 0;
    }

    // Process Butterworth forward and backward
    pState->low->processForwardAndReverse(pState->m_pProcessBuf,
                                          pState->m_pLowBuf, numSamples * 2 + 3000);

    //qDebug() << "#################";
    //for (unsigned int i = 0; i < numSamples / 2 + 1500; ++i) {
    //    qDebug() << i << pState->m_pLowBuf[i*2];
    //}

    // Process Butterworth forward and backward
    pState->high->processForwardAndReverse(pState->m_pProcessBuf,
                                           pState->m_pHighBuf, numSamples * 2 + 3000);

    // No raming is required here since we are in a Hanning Window
    SampleUtil::copy3WithGain(pState->m_pProcessBuf,
            pState->m_pProcessBuf, fLow,
            pState->m_pLowBuf, fMid,
            pState->m_pHighBuf, fHigh,
            numSamples * 2);

    // Coppy second half of old data to output
    for (unsigned int i = 0; i < numSamples; ++i) {
        pOutput[i] = pState->m_pLastProcessBuf[i + numSamples];
    }

    // Safe process Buffer
    for (unsigned int i = 0; i < numSamples * 2; ++i) {
        pState->m_pLastProcessBuf[i] = pState->m_pProcessBuf[i];
    }

    // copy top destination Buffer with overlapping.
    for (unsigned int i = 0; i < numSamples; ++i) {
        pOutput[i] += pState->m_pLastProcessBuf[i];
    }
}
