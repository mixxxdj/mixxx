#include "effects/native/graphiceqeffect.h"
#include "util/math.h"

#define Q 1.2247449

// static
QString GraphicEQEffect::getId() {
    return "org.mixxx.effects.graphiceq";
}

// static
EffectManifest GraphicEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Graphic EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "An 8 band Graphic EQ based on Biquad Filters"));

    // Display center frequencies for each filter
    float centerFrequencies[8] = {31.25, 62.5, 125, 250, 500, 1000,
                                  2000, 4000};

    EffectManifestParameter* low = manifest.addParameter();
    low->setId(QString("low"));
    low->setName(QString("31.25 Hz"));
    low->setDescription(QString("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setDefault(0);
    low->setMinimum(-12);
    low->setMaximum(12);

    QString paramName;
    for (int i = 0; i < 6; i++) {
        if (centerFrequencies[i] < 1000) {
            paramName = QString("%1 Hz").arg(centerFrequencies[i + 1]);
        } else {
            paramName = QString("%1 kHz").arg(centerFrequencies[i + 1] / 1000);
        }

        EffectManifestParameter* mid = manifest.addParameter();
        mid->setId(QString("mid%1").arg(i));
        mid->setName(paramName);
        mid->setDescription(QString("Gain for Band Filter %1").arg(i));
        mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
        mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
        mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
        mid->setDefault(0);
        mid->setMinimum(-12);
        mid->setMaximum(12);
    }

    EffectManifestParameter* high = manifest.addParameter();
    high->setId(QString("high"));
    high->setName(QString("4 kHz"));
    high->setDescription(QString("Gain for Hight Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    high->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(0);
    high->setMinimum(-12);
    high->setMaximum(12);

    return manifest;
}

GraphicEQEffectGroupState::GraphicEQEffectGroupState() {
    m_oldLow = 0;
    for (int i = 0; i < 6; i++) {
        m_oldMid.append(1.0);
    }
    m_oldHigh = 0;

    m_pBuf1 = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBuf2 = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize the default center frequencies
    m_centerFrequencies[0] = 31.25;
    m_centerFrequencies[1] = 62.5;
    m_centerFrequencies[2] = 125.0;
    m_centerFrequencies[3] = 250.0;
    m_centerFrequencies[4] = 500.0;
    m_centerFrequencies[5] = 1000.0;
    m_centerFrequencies[6] = 2000.0;
    m_centerFrequencies[7] = 4000.0;

    // TODO(rryan): use the real samplerate
    // Initialize the filters with default parameters
    m_low = new EngineFilterBiquad1Low(44100, m_centerFrequencies[0], Q);
    m_high = new EngineFilterBiquad1High(44100, m_centerFrequencies[7], Q);
    for (int i = 1; i < 7; i++) {
        m_bands.append(new EngineFilterBiquad1Band(44100,
                                                   m_centerFrequencies[i], Q));
    }
}

GraphicEQEffectGroupState::~GraphicEQEffectGroupState() {
    foreach (EngineFilterBiquad1Band* filter, m_bands) {
        delete filter;
    }
    SampleUtil::free(m_pBuf1);
    SampleUtil::free(m_pBuf2);
}

void GraphicEQEffectGroupState::setFilters(int sampleRate) {
    m_low->setFrequencyCorners(sampleRate, m_centerFrequencies[0], Q,
                               m_oldLow);
    m_high->setFrequencyCorners(sampleRate, m_centerFrequencies[7], Q,
                                m_oldHigh);
    for (int i = 0; i < 6; i++) {
        m_bands[i]->setFrequencyCorners(sampleRate, m_centerFrequencies[i + 1],
                                        Q, m_oldMid[i]);
    }
}

GraphicEQEffect::GraphicEQEffect(EngineEffect* pEffect,
                                 const EffectManifest& manifest)
        : m_oldSampleRate(44100) {
    Q_UNUSED(manifest);
    m_pPotLow = pEffect->getParameterById("low");
    for (int i = 0; i < 6; i++) {
        m_pPotMid.append(pEffect->getParameterById(QString("mid%1").arg(i)));
    }
    m_pPotHigh = pEffect->getParameterById("high");
}

GraphicEQEffect::~GraphicEQEffect() {
}

void GraphicEQEffect::processGroup(const QString& group,
                                   GraphicEQEffectGroupState* pState,
                                   const CSAMPLE* pInput, CSAMPLE* pOutput,
                                   const unsigned int numSamples,
                                   const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    int sampleRate = getSampleRate();
    if (m_oldSampleRate != sampleRate) {
        m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate);
    }

    float fLow;
    float fMid[6];
    float fHigh;

    fLow = m_pPotLow->value().toDouble();
    fHigh = m_pPotHigh->value().toDouble();
    for (int i = 0; i < 6; i++) {
        fMid[i] = m_pPotMid[i]->value().toDouble();
    }


    if (fLow != pState->m_oldLow) {
        pState->m_low->setFrequencyCorners(sampleRate, pState->m_centerFrequencies[0], Q,
                                           fLow);
    }
    if (fHigh != pState->m_oldHigh) {
        pState->m_high->setFrequencyCorners(sampleRate, pState->m_centerFrequencies[7], Q,
                                            fHigh);
    }
    for (int i = 0; i < 6; i++) {
        if (fMid[i] != pState->m_oldMid[i]) {
            pState->m_bands[i]->setFrequencyCorners(sampleRate, pState->m_centerFrequencies[i + 1],
                                                    Q, fMid[i]);
        }
    }

    pState->m_low->process(pInput, pState->m_pBuf1, numSamples);
    pState->m_bands[0]->process(pState->m_pBuf1, pState->m_pBuf2, numSamples);
    pState->m_bands[1]->process(pState->m_pBuf2, pState->m_pBuf1, numSamples);
    pState->m_bands[2]->process(pState->m_pBuf1, pState->m_pBuf2, numSamples);
    pState->m_bands[3]->process(pState->m_pBuf2, pState->m_pBuf1, numSamples);
    pState->m_bands[4]->process(pState->m_pBuf1, pState->m_pBuf2, numSamples);
    pState->m_bands[5]->process(pState->m_pBuf2, pState->m_pBuf1, numSamples);
    pState->m_high->process(pState->m_pBuf1, pOutput, numSamples);

    pState->m_oldLow = fLow;
    pState->m_oldHigh = fHigh;
    for (int i = 0; i < 6; i++) {
        pState->m_oldMid[i] = fMid[i];
    }

}
