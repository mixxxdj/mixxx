#include "effects/backends/builtin/graphiceqeffect.h"

#include "audio/types.h"
#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"

namespace {
constexpr double kQ = 1.2247449;
} // namespace

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
    pManifest->setIsMainEQ(true);

    // Display rounded center frequencies for each filter
    float centerFrequencies[8] = {45, 100, 220, 500, 1100, 2500, 5500, 12000};

    EffectManifestParameterPointer low = pManifest->addParameter();
    low->setId(QString("low"));
    low->setName(QString("%1 Hz").arg(centerFrequencies[0]));
    low->setShortName(QString("%1 Hz").arg(centerFrequencies[0]));
    low->setDescription(QObject::tr(
            "Gain for Low Filter"));
    low->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    low->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    low->setNeutralPointOnScale(0.5);
    low->setRange(-12, 0, 12);

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
                "Gain for Band Filter %1")
                                    .arg(i + 1));
        mid->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
        mid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        mid->setNeutralPointOnScale(0.5);
        mid->setRange(-12, 0, 12);
    }

    EffectManifestParameterPointer high = pManifest->addParameter();
    high->setId(QString("high"));
    high->setName(QString("%1 kHz").arg(centerFrequencies[7] / 1000));
    high->setDescription(QObject::tr(
            "Gain for High Filter"));
    high->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    high->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    high->setNeutralPointOnScale(0.5);
    high->setRange(-12, 0, 12);

    return pManifest;
}

GraphicEQEffectGroupState::GraphicEQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters) {
    m_oldLow = 0;
    for (int i = 0; i < 6; i++) {
        m_oldMid.append(1.0);
    }
    m_oldHigh = 0;

    m_pBufs.append(SampleUtil::alloc(engineParameters.samplesPerBuffer()));
    m_pBufs.append(SampleUtil::alloc(engineParameters.samplesPerBuffer()));

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
    m_low = new EngineFilterBiquad1LowShelving(
            engineParameters.sampleRate(),
            m_centerFrequencies[0],
            kQ);
    m_high = new EngineFilterBiquad1HighShelving(
            engineParameters.sampleRate(),
            m_centerFrequencies[7],
            kQ);
    for (int i = 1; i < 7; i++) {
        m_bands.append(new EngineFilterBiquad1Peaking(engineParameters.sampleRate(),
                m_centerFrequencies[i],
                kQ));
    }
}

GraphicEQEffectGroupState::~GraphicEQEffectGroupState() {
    foreach (EngineFilterBiquad1Peaking* filter, m_bands) {
        delete filter;
    }

    delete m_low;
    delete m_high;

    foreach (CSAMPLE* buf, m_pBufs) {
        SampleUtil::free(buf);
    }
}

void GraphicEQEffectGroupState::setFilters(mixxx::audio::SampleRate sampleRate) {
    m_low->setFrequencyCorners(sampleRate, m_centerFrequencies[0], kQ, m_oldLow);
    m_high->setFrequencyCorners(sampleRate, m_centerFrequencies[7], kQ, m_oldHigh);
    for (int i = 0; i < 6; i++) {
        m_bands[i]->setFrequencyCorners(sampleRate, m_centerFrequencies[i + 1], kQ, m_oldMid[i]);
    }
}

void GraphicEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    for (int i = 0; i < 6; i++) {
        m_pPotMid.append(parameters.value(QString("mid%1").arg(i)));
    }
    m_pPotHigh = parameters.value("high");
}

void GraphicEQEffect::processChannel(
        GraphicEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    if (m_oldSampleRate != engineParameters.sampleRate()) {
        m_oldSampleRate = engineParameters.sampleRate();
        pState->setFilters(engineParameters.sampleRate());
    }

    float fLow;
    float fMid[6];
    float fHigh;

    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        fLow = 1.0;
        fHigh = 1.0;
        ;
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
        pState->m_low->setFrequencyCorners(engineParameters.sampleRate(),
                pState->m_centerFrequencies[0],
                kQ,
                fLow);
    }
    if (fHigh != pState->m_oldHigh) {
        pState->m_high->setFrequencyCorners(engineParameters.sampleRate(),
                pState->m_centerFrequencies[7],
                kQ,
                fHigh);
    }
    for (int i = 0; i < 6; i++) {
        if (fMid[i] != pState->m_oldMid[i]) {
            pState->m_bands[i]->setFrequencyCorners(engineParameters.sampleRate(),
                    pState->m_centerFrequencies[i + 1],
                    kQ,
                    fMid[i]);
        }
    }

    int bufIndex = 0;
    if (fLow != 0) {
        pState->m_low->process(pInput,
                pState->m_pBufs[1 - bufIndex],
                engineParameters.samplesPerBuffer());
        bufIndex = 1 - bufIndex;
    } else {
        pState->m_low->pauseFilter();
        SampleUtil::copy(pState->m_pBufs[bufIndex], pInput, engineParameters.samplesPerBuffer());
    }

    for (int i = 0; i < 6; i++) {
        if (fMid[i] != 0) {
            pState->m_bands[i]->process(pState->m_pBufs[bufIndex],
                    pState->m_pBufs[1 - bufIndex],
                    engineParameters.samplesPerBuffer());
            bufIndex = 1 - bufIndex;
        } else {
            pState->m_bands[i]->pauseFilter();
        }
    }

    if (fHigh != 0) {
        pState->m_high->process(pState->m_pBufs[bufIndex],
                pOutput,
                engineParameters.samplesPerBuffer());
    } else {
        SampleUtil::copy(pOutput, pState->m_pBufs[bufIndex], engineParameters.samplesPerBuffer());
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
