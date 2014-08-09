#include "effects/native/graphiceqeffect.h"
#include "util/math.h"

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
        "A 10 band Graphic EQ based on Biquad Filters"));

    // Display center frequencies for each filter
    float centerFrequencies[10] = {31.25, 62.5, 125, 250, 500, 1000,
                                   2000, 4000, 8000, 16000};
    QString paramName;
    for (int i = 0; i < 10; i++) {
        if (centerFrequencies[i] < 1000) {
            paramName = QString("%1 Hz").arg(centerFrequencies[i]);
        } else {
            paramName = QString("%1 kHz").arg(centerFrequencies[i] / 1000);
        }

        EffectManifestParameter* mid = manifest.addParameter();
        mid->setId(QString("mid%1").arg(i));
        mid->setName(paramName);
        mid->setDescription(QString("Gain for Band Filter %1").arg(i));
        mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
        mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
        mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
        mid->setDefault(1.0);
        mid->setMinimum(0);
        mid->setMaximum(4.0);
    }

    return manifest;
}

GraphicEQEffectGroupState::GraphicEQEffectGroupState() {
    for (int i = 0; i < 10; i++) {
        m_pBandBuf.append(SampleUtil::alloc(MAX_BUFFER_LEN));
        m_oldMid.append(1.0);
    }

    // Initialize the default center frequencies
    m_centerFrequencies[0] = 31.25;
    m_centerFrequencies[1] = 62.5;
    m_centerFrequencies[2] = 125.0;
    m_centerFrequencies[3] = 250.0;
    m_centerFrequencies[4] = 500.0;
    m_centerFrequencies[5] = 1000.0;
    m_centerFrequencies[6] = 2000.0;
    m_centerFrequencies[7] = 4000.0;
    m_centerFrequencies[8] = 8000.0;
    m_centerFrequencies[9] = 16000.0;

    // TODO(rryan): use the real samplerate
    setFilters(44100, 2);
}

GraphicEQEffectGroupState::~GraphicEQEffectGroupState() {
    foreach (EngineFilterBiquad1Band* filter, m_bands) {
        delete filter;
    }
    foreach (CSAMPLE* buf, m_pBandBuf) {
        SampleUtil::free(buf);
    }
}

void GraphicEQEffectGroupState::setFilters(int sampleRate, double Q) {
    for (int i = 0; i < 10; i++) {
        m_bands.append(new EngineFilterBiquad1Band(sampleRate,
                                                m_centerFrequencies[i], Q));
    }
}

GraphicEQEffect::GraphicEQEffect(EngineEffect* pEffect,
                                 const EffectManifest& manifest)
        : m_oldSampleRate(44100) {
    Q_UNUSED(manifest);
    for (int i = 0; i < 10; i++) {
        m_pPotMid.append(pEffect->getParameterById(QString("mid%1").arg(i)));
    }
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
        pState->setFilters(sampleRate, 2);
    }

    float fMid[10];
    for (int i = 0; i < 10; i++) {
        fMid[i] = m_pPotMid[i]->value().toDouble();
    }

    for (int i = 0; i < 10; i++) {
        if (fMid[i] || pState->m_oldMid[i]) {
            pState->m_bands[i]->process(pInput, pState->m_pBandBuf[i], numSamples);
        } else {
            pState->m_bands[i]->pauseFilter();
        }
    }

    if (fMid[0] != pState->m_oldMid[0] ||
        fMid[1] != pState->m_oldMid[1] ||
        fMid[2] != pState->m_oldMid[2] ||
        fMid[3] != pState->m_oldMid[3] ||
        fMid[4] != pState->m_oldMid[4] ||
        fMid[5] != pState->m_oldMid[5] ||
        fMid[6] != pState->m_oldMid[6] ||
        fMid[7] != pState->m_oldMid[7] ||
        fMid[8] != pState->m_oldMid[8] ||
        fMid[9] != pState->m_oldMid[9]) {
        SampleUtil::copy10WithRampingGain(pOutput,
                pState->m_pBandBuf[0], pState->m_oldMid[0], fMid[0],
                pState->m_pBandBuf[1], pState->m_oldMid[1], fMid[1],
                pState->m_pBandBuf[2], pState->m_oldMid[2], fMid[2],
                pState->m_pBandBuf[3], pState->m_oldMid[3], fMid[3],
                pState->m_pBandBuf[4], pState->m_oldMid[4], fMid[4],
                pState->m_pBandBuf[5], pState->m_oldMid[5], fMid[5],
                pState->m_pBandBuf[6], pState->m_oldMid[6], fMid[6],
                pState->m_pBandBuf[7], pState->m_oldMid[7], fMid[7],
                pState->m_pBandBuf[8], pState->m_oldMid[8], fMid[8],
                pState->m_pBandBuf[9], pState->m_oldMid[9], fMid[9],
                numSamples);
    } else {
        SampleUtil::copy10WithGain(pOutput,
                pState->m_pBandBuf[0], fMid[0],
                pState->m_pBandBuf[1], fMid[1],
                pState->m_pBandBuf[2], fMid[2],
                pState->m_pBandBuf[3], fMid[3],
                pState->m_pBandBuf[4], fMid[4],
                pState->m_pBandBuf[5], fMid[5],
                pState->m_pBandBuf[6], fMid[6],
                pState->m_pBandBuf[7], fMid[7],
                pState->m_pBandBuf[8], fMid[8],
                pState->m_pBandBuf[9], fMid[9],
                numSamples);
    }

    for (int i = 0; i < 10; i++) {
        pState->m_oldMid[i] = fMid[i];
    }
}
