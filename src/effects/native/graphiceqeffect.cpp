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
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMasterEQ(true);

    // Display rounded center frequencies for each filter
    float centerFrequencies[8] = {45, 100, 220, 500, 1100, 2500,
                                  5500, 12000};

    EffectManifestParameter* low = manifest.addParameter();
    low->setId(QString("low"));
    low->setName(QString("%1 Hz").arg(centerFrequencies[0]));
    low->setDescription(QString("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
    low->setDefault(0);
    low->setMinimum(-12);
    low->setMaximum(12);

    QString paramName;
    for (int i = 0; i < 6; i++) {
        if (centerFrequencies[i + 1] < 1000) {
            paramName = QString("%1 Hz").arg(centerFrequencies[i + 1]);
        } else {
            paramName = QString("%1 kHz").arg(centerFrequencies[i + 1] / 1000);
        }

        EffectManifestParameter* mid = manifest.addParameter();
        mid->setId(QString("mid%1").arg(i));
        mid->setName(paramName);
        mid->setDescription(QString("Gain for Band Filter %1").arg(i));
        mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
        mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
        mid->setNeutralPointOnScale(0.5);
        mid->setDefault(0);
        mid->setMinimum(-12);
        mid->setMaximum(12);
    }

    EffectManifestParameter* high = manifest.addParameter();
    high->setId(QString("high"));
    high->setName(QString("%1 kHz").arg(centerFrequencies[7] / 1000));
    high->setDescription(QString("Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
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

    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));

    // Initialize the default center frequencies
    m_centerFrequencies[0] = 81;
    m_centerFrequencies[1] = 100;
    m_centerFrequencies[2] = 222;
    m_centerFrequencies[3] = 494;
    m_centerFrequencies[4] = 1097;
    m_centerFrequencies[5] = 2437;
    m_centerFrequencies[6] = 5416;
    m_centerFrequencies[7] = 9828;

    // Initialize the filters with default parameters
    m_low = new EngineFilterBiquad1LowShelving(44100, m_centerFrequencies[0], Q);
    m_high = new EngineFilterBiquad1HighShelving(44100, m_centerFrequencies[7], Q);
    for (int i = 1; i < 7; i++) {
        m_bands.append(new EngineFilterBiquad1Peaking(44100,
                                                      m_centerFrequencies[i],
                                                      Q));
    }
}

GraphicEQEffectGroupState::~GraphicEQEffectGroupState() {
    foreach (EngineFilterBiquad1Peaking* filter, m_bands) {
        delete filter;
    }

    foreach(CSAMPLE* buf, m_pBufs) {
        SampleUtil::free(buf);
    }
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
                                   const unsigned int sampleRate,
                                   const EffectProcessor::EnableState enableState,
                                   const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    if (m_oldSampleRate != sampleRate) {
        m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate);
    }

    float fLow;
    float fMid[6];
    float fHigh;

    if (enableState == EffectProcessor::DISABLING) {
         // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        fLow = 1.0;
        fHigh = 1.0;;
        for (int i = 0; i < 6; i++) {
            fMid[i] = 1.0;
        }
    } else {
        fLow = m_pPotLow->value();
        fHigh = m_pPotHigh->value();
        for (int i = 0; i < 6; i++) {
            fMid[i] = m_pPotMid[i]->value();
        }
    }


    if (fLow != pState->m_oldLow) {
        pState->m_low->setFrequencyCorners(sampleRate,
                                           pState->m_centerFrequencies[0], Q,
                                           fLow);
    }
    if (fHigh != pState->m_oldHigh) {
        pState->m_high->setFrequencyCorners(sampleRate,
                                            pState->m_centerFrequencies[7], Q,
                                            fHigh);
    }
    for (int i = 0; i < 6; i++) {
        if (fMid[i] != pState->m_oldMid[i]) {
            pState->m_bands[i]->setFrequencyCorners(sampleRate,
                                                    pState->m_centerFrequencies[i + 1],
                                                    Q, fMid[i]);
        }
    }

    int bufIndex = 0;
    if (fLow) {
        pState->m_low->process(pInput, pState->m_pBufs[1 - bufIndex], numSamples);
        bufIndex = 1 - bufIndex;
    } else {
        pState->m_low->pauseFilter();
        SampleUtil::copy(pState->m_pBufs[bufIndex], pInput, numSamples);
    }

    for (int i = 0; i < 6; i++) {
        if (fMid[i]) {
            pState->m_bands[i]->process(pState->m_pBufs[bufIndex],
                                        pState->m_pBufs[1 - bufIndex], numSamples);
            bufIndex = 1 - bufIndex;
        } else {
            pState->m_bands[i]->pauseFilter();
        }
    }

    if (fHigh) {
        pState->m_high->process(pState->m_pBufs[bufIndex],
                                pOutput, numSamples);
        bufIndex = 1 - bufIndex;
    } else {
        SampleUtil::copy(pOutput, pState->m_pBufs[bufIndex], numSamples);
        pState->m_high->pauseFilter();
    }


    pState->m_oldLow = fLow;
    pState->m_oldHigh = fHigh;
    for (int i = 0; i < 6; i++) {
        pState->m_oldMid[i] = fMid[i];
    }

    if (enableState == EffectProcessor::DISABLING) {
        pState->m_low->pauseFilter();
        pState->m_high->pauseFilter();
        for (int i = 0; i < 6; i++) {
            pState->m_bands[i]->pauseFilter();
        }
    }
}
