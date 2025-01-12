#include "effects/backends/builtin/parametriceqeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

namespace {
constexpr int kBandCount = 2;
constexpr double kDefaultCenter1 = 1000; // 1 kHz
constexpr double kDefaultCenter2 = 3000; // 3 kHz
} // namespace

// static
QString ParametricEQEffect::getId() {
    return "org.mixxx.effects.parametriceq";
}

// static
EffectManifestPointer ParametricEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Parametric Equalizer"));
    pManifest->setShortName(QObject::tr("Param EQ"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "A gentle 2-band parametric equalizer based on biquad filters.\n"
            "It is designed as a complement to the steep mixing equalizers."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMainEQ(true);

    EffectManifestParameterPointer gain1 = pManifest->addParameter();
    gain1->setId("gain1");
    gain1->setName(QObject::tr("Gain 1"));
    gain1->setShortName(QObject::tr("Gain 1"));
    gain1->setDescription(QObject::tr(
            "Gain for Filter 1"));
    gain1->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain1->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain1->setNeutralPointOnScale(0.5);
    gain1->setRange(-18, 0, 18); // dB

    EffectManifestParameterPointer q1 = pManifest->addParameter();
    q1->setId("q1");
    q1->setName(QObject::tr("Q 1"));
    q1->setShortName(QObject::tr("Q 1"));
    q1->setDescription(QObject::tr(
            "Controls the bandwidth of Filter 1.\n"
            "A lower Q affects a wider band of frequencies,\n"
            "a higher Q affects a narrower band of frequencies."));
    q1->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    q1->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    q1->setNeutralPointOnScale(0.5);
    q1->setRange(0.5, 1.75, 3.0);

    EffectManifestParameterPointer center1 = pManifest->addParameter();
    center1->setId("center1");
    center1->setName(QObject::tr("Center 1"));
    center1->setShortName(QObject::tr("Center 1"));
    center1->setDescription(QObject::tr(
            "Center frequency for Filter 1, from 100 Hz to 14 kHz"));
    center1->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    center1->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    center1->setRange(100, kDefaultCenter1, 14000);

    EffectManifestParameterPointer gain2 = pManifest->addParameter();
    gain2->setId("gain2");
    gain2->setName(QObject::tr("Gain 2"));
    gain2->setShortName(QObject::tr("Gain 2"));
    gain2->setDescription(QObject::tr(
            "Gain for Filter 2"));
    gain2->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain2->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain2->setNeutralPointOnScale(0.5);
    gain2->setRange(-18, 0, 18); // dB

    EffectManifestParameterPointer q2 = pManifest->addParameter();
    q2->setId("q2");
    q2->setName(QObject::tr("Q 2"));
    q2->setShortName(QObject::tr("Q 2"));
    q2->setDescription(QObject::tr(
            "Controls the bandwidth of Filter 2.\n"
            "A lower Q affects a wider band of frequencies,\n"
            "a higher Q affects a narrower band of frequencies."));
    q2->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    q2->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    q2->setNeutralPointOnScale(0.5);
    q2->setRange(0.5, 1.75, 3.0);

    EffectManifestParameterPointer center2 = pManifest->addParameter();
    center2->setId("center2");
    center2->setName(QObject::tr("Center 2"));
    center2->setShortName(QObject::tr("Center 2"));
    center2->setDescription(QObject::tr(
            "Center frequency for Filter 2, from 100 Hz to 14 kHz"));
    center2->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    center2->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    center2->setRange(100, kDefaultCenter2, 14000);

    return pManifest;
}

ParametricEQEffectGroupState::ParametricEQEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_oldSampleRate(44100) {
    for (int i = 0; i < kBandCount; i++) {
        m_oldGain.append(1.0);
        m_oldQ.append(1.75);
    }

    m_oldCenter.append(kDefaultCenter1);
    m_oldCenter.append(kDefaultCenter2);

    // Initialize the filters with default parameters
    for (int i = 0; i < kBandCount; i++) {
        m_bands.push_back(std::make_unique<EngineFilterBiquad1Peaking>(
                engineParameters.sampleRate(), m_oldCenter[i], m_oldQ[i]));
    }
}

void ParametricEQEffectGroupState::setFilters(mixxx::audio::SampleRate sampleRate) {
    for (int i = 0; i < kBandCount; i++) {
        m_bands[i]->setFrequencyCorners(
                sampleRate, m_oldCenter[i], m_oldQ[i], m_oldGain[i]);
    }
}

void ParametricEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotGain.append(parameters.value("gain1"));
    m_pPotQ.append(parameters.value("q1"));
    m_pPotCenter.append(parameters.value("center1"));
    m_pPotGain.append(parameters.value("gain2"));
    m_pPotQ.append(parameters.value("q2"));
    m_pPotCenter.append(parameters.value("center2"));
}

void ParametricEQEffect::processChannel(
        ParametricEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    if (pState->m_oldSampleRate != engineParameters.sampleRate()) {
        pState->m_oldSampleRate = engineParameters.sampleRate();
        pState->setFilters(engineParameters.sampleRate());
    }

    CSAMPLE_GAIN fGain[2];
    CSAMPLE_GAIN fQ[2];
    CSAMPLE_GAIN fCenter[2];

    for (int i = 0; i < kBandCount; i++) {
        if (enableState == EffectEnableState::Disabling) {
            // Ramp to dry, when disabling, this will ramp from dry when enabling as well
            fGain[i] = 1.0;
        } else {
            fGain[i] = static_cast<CSAMPLE_GAIN>(m_pPotGain[i]->value());
        }
        fQ[i] = static_cast<CSAMPLE_GAIN>(m_pPotQ[i]->value());
        fCenter[i] = static_cast<CSAMPLE_GAIN>(m_pPotCenter[i]->value());
        if (fGain[i] != pState->m_oldGain[i] ||
                fQ[i] != pState->m_oldQ[i] ||
                fCenter[i] != pState->m_oldCenter[i]) {
            pState->m_bands[i]->setFrequencyCorners(
                    engineParameters.sampleRate(), fCenter[i], fQ[i], fGain[i]);
        }
    }

    if (fGain[0] != 0) {
        pState->m_bands[0]->process(pInput, pOutput, engineParameters.samplesPerBuffer());
        if (fGain[1] != 0) {
            pState->m_bands[1]->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
        } else {
            pState->m_bands[1]->pauseFilter();
        }
    } else {
        pState->m_bands[0]->pauseFilter();
        if (fGain[1] != 0) {
            pState->m_bands[1]->process(pInput, pOutput, engineParameters.samplesPerBuffer());
        } else {
            pState->m_bands[1]->pauseFilter();
            if (pOutput != pInput) {
                SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
            }
        }
    }

    for (int i = 0; i < kBandCount; i++) {
        pState->m_oldGain[i] = fGain[i];
        pState->m_oldQ[i] = fQ[i];
        pState->m_oldCenter[i] = fCenter[i];
    }

    if (enableState == EffectEnableState::Disabling) {
        for (int i = 0; i < kBandCount; i++) {
            pState->m_bands[i]->pauseFilter();
        }
    }
}
