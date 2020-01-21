#include "effects/builtin/graphiceqeffect.h"
#include "util/math.h"

#define Q 1.2247449

// static
QString GraphicEQEffect::getId() {
    return "org.mixxx.effects.graphiceq";
}

// static
EffectManifestPointer GraphicEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Graphic Equalizer"));
    pManifest->setShortName(QObject::tr("Graphic EQ"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
        "An 8-band graphic equalizer based on biquad filters"));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMasterEQ(true);

    // Display rounded center frequencies for each filter
    float centerFrequencies[8] = {45, 100, 220, 500, 1100, 2500,
                                  5500, 12000};

    EffectManifestParameterPointer low = pManifest->addParameter();
    low->setId(QString("low"));
    low->setName(QString("%1 Hz").arg(centerFrequencies[0]));
    low->setShortName(QString("%1 Hz").arg(centerFrequencies[0]));
    low->setDescription(QObject::tr(
        "Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    low->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
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

        EffectManifestParameterPointer mid = pManifest->addParameter();
        mid->setId(QString("mid%1").arg(i));
        mid->setName(paramName);
        mid->setShortName(paramName);
        mid->setDescription(QObject::tr(
            "Gain for Band Filter %1").arg(i + 1));
        mid->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
        mid->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        mid->setNeutralPointOnScale(0.5);
        mid->setDefault(0);
        mid->setMinimum(-12);
        mid->setMaximum(12);
    }

    EffectManifestParameterPointer high = pManifest->addParameter();
    high->setId(QString("high"));
    high->setName(QString("%1 kHz").arg(centerFrequencies[7] / 1000));
    high->setDescription(QObject::tr(
        "Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    high->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    high->setDefault(0);
    high->setMinimum(-12);
    high->setMaximum(12);

    return pManifest;
}

GraphicEQEffectGroupState::GraphicEQEffectGroupState(
        const mixxx::EngineParameters& bufferParameters)
            : EffectState(bufferParameters) {
    m_oldLow = 0;
    for (int i = 0; i < 6; i++) {
        m_oldMid.append(1.0);
    }
    m_oldHigh = 0;

    m_pBufs.append(SampleUtil::alloc(bufferParameters.samplesPerBuffer()));
    m_pBufs.append(SampleUtil::alloc(bufferParameters.samplesPerBuffer()));

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

    delete m_low;
    delete m_high;

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

GraphicEQEffect::GraphicEQEffect(EngineEffect* pEffect)
        : m_oldSampleRate(44100) {
    m_pPotLow = pEffect->getParameterById("low");
    for (int i = 0; i < 6; i++) {
        m_pPotMid.append(pEffect->getParameterById(QString("mid%1").arg(i)));
    }
    m_pPotHigh = pEffect->getParameterById("high");
}

GraphicEQEffect::~GraphicEQEffect() {
}

void GraphicEQEffect::processChannel(const ChannelHandle& handle,
                                     GraphicEQEffectGroupState* pState,
                                     const CSAMPLE* pInput, CSAMPLE* pOutput,
                                     const mixxx::EngineParameters& bufferParameters,
                                     const EffectEnableState enableState,
                                     const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    if (m_oldSampleRate != bufferParameters.sampleRate()) {
        m_oldSampleRate = bufferParameters.sampleRate();
        pState->setFilters(bufferParameters.sampleRate());
    }

    float fLow;
    float fMid[6];
    float fHigh;

    if (enableState == EffectEnableState::Disabling) {
         // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        fLow = 1.0;
        fHigh = 1.0;;
        for (int i = 0; i < 6; i++) {
            fMid[i] = 1.0;
        }
    } else {
        fLow = static_cast<float>(m_pPotLow->value());
        fHigh = static_cast<float>(m_pPotHigh->value());
        for (int i = 0; i < 6; i++) {
            fMid[i] = static_cast<float>(m_pPotMid[i]->value());
        }
    }


    if (fLow != pState->m_oldLow) {
        pState->m_low->setFrequencyCorners(bufferParameters.sampleRate(),
                                           pState->m_centerFrequencies[0], Q,
                                           fLow);
    }
    if (fHigh != pState->m_oldHigh) {
        pState->m_high->setFrequencyCorners(bufferParameters.sampleRate(),
                                            pState->m_centerFrequencies[7], Q,
                                            fHigh);
    }
    for (int i = 0; i < 6; i++) {
        if (fMid[i] != pState->m_oldMid[i]) {
            pState->m_bands[i]->setFrequencyCorners(bufferParameters.sampleRate(),
                                                    pState->m_centerFrequencies[i + 1],
                                                    Q, fMid[i]);
        }
    }

    int bufIndex = 0;
    if (fLow != 0) {
        pState->m_low->process(pInput, pState->m_pBufs[1 - bufIndex], bufferParameters.samplesPerBuffer());
        bufIndex = 1 - bufIndex;
    } else {
        pState->m_low->pauseFilter();
        SampleUtil::copy(pState->m_pBufs[bufIndex], pInput, bufferParameters.samplesPerBuffer());
    }

    for (int i = 0; i < 6; i++) {
        if (fMid[i] != 0) {
            pState->m_bands[i]->process(pState->m_pBufs[bufIndex],
                                        pState->m_pBufs[1 - bufIndex], bufferParameters.samplesPerBuffer());
            bufIndex = 1 - bufIndex;
        } else {
            pState->m_bands[i]->pauseFilter();
        }
    }

    if (fHigh != 0) {
        pState->m_high->process(pState->m_pBufs[bufIndex],
                                pOutput, bufferParameters.samplesPerBuffer());
    } else {
        SampleUtil::copy(pOutput, pState->m_pBufs[bufIndex], bufferParameters.samplesPerBuffer());
        pState->m_high->pauseFilter();
    }

    pState->m_oldLow = fLow;
    pState->m_oldHigh = fHigh;
    for (int i = 0; i < 6; i++) {
        pState->m_oldMid[i] = fMid[i];
    }

    if (enableState == EffectEnableState::Disabling) {
        pState->m_low->pauseFilter();
        pState->m_high->pauseFilter();
        for (int i = 0; i < 6; i++) {
            pState->m_bands[i]->pauseFilter();
        }
    }
}
