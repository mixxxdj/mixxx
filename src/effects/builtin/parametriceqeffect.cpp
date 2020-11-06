#include "effects/builtin/parametriceqeffect.h"
#include "util/math.h"

namespace {
    constexpr int kBandCount = 2;
    constexpr double kDefaultCenter1 = 1000; // 1 kHz
    constexpr double kDefaultCenter2 = 3000; // 3 kHz
}

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
        "An gentle 2-band parametric equalizer based on biquad filters.\n"
        "It is designed as a complement to the steep mixing equalizers."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMasterEQ(true);

    EffectManifestParameterPointer gain1 = pManifest->addParameter();
    gain1->setId("gain1");
    gain1->setName(QObject::tr("Gain 1"));
    gain1->setShortName(QObject::tr("Gain 1"));
    gain1->setDescription(QObject::tr(
        "Gain for Filter 1"));
    gain1->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    gain1->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    gain1->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    gain1->setNeutralPointOnScale(0.5);
    gain1->setDefault(0);
    gain1->setMinimum(-18);
    gain1->setMaximum(18); // dB

    EffectManifestParameterPointer q1 = pManifest->addParameter();
    q1->setId("q1");
    q1->setName(QObject::tr("Q 1"));
    q1->setShortName(QObject::tr("Q 1"));
    q1->setDescription(QObject::tr(
        "Controls the bandwidth of Filter 1.\n"
        "A lower Q affects a wider band of frequencies,\n"
        "a higher Q affects a narrower band of frequencies."));
    q1->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    q1->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    q1->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    q1->setNeutralPointOnScale(0.5);
    q1->setDefault(1.75);
    q1->setMinimum(0.5);
    q1->setMaximum(3.0);

    EffectManifestParameterPointer center1 = pManifest->addParameter();
    center1->setId("center1");
    center1->setName(QObject::tr("Center 1"));
    center1->setShortName(QObject::tr("Center 1"));
    center1->setDescription(QObject::tr(
        "Center frequency for Filter 1, from 100 Hz to 14 kHz"));
    center1->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    center1->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    center1->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    center1->setNeutralPointOnScale(0.5);
    center1->setDefault(kDefaultCenter1);
    center1->setMinimum(100);
    center1->setMaximum(14000);

    EffectManifestParameterPointer gain2 = pManifest->addParameter();
    gain2->setId("gain2");
    gain2->setName(QObject::tr("Gain 2"));
    gain2->setShortName(QObject::tr("Gain 2"));
    gain2->setDescription(QObject::tr(
        "Gain for Filter 2"));
    gain2->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    gain2->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    gain2->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    gain2->setNeutralPointOnScale(0.5);
    gain2->setDefault(0);
    gain2->setMinimum(-18);
    gain2->setMaximum(18); // dB

    EffectManifestParameterPointer q2 = pManifest->addParameter();
    q2->setId("q2");
    q2->setName(QObject::tr("Q 2"));
    q2->setShortName(QObject::tr("Q 2"));
    q2->setDescription(QObject::tr(
        "Controls the bandwidth of Filter 2.\n"
        "A lower Q affects a wider band of frequencies,\n"
        "a higher Q affects a narrower band of frequencies."));
    q2->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    q2->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    q2->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    q2->setNeutralPointOnScale(0.5);
    q2->setDefault(1.75);
    q2->setMinimum(0.5);
    q2->setMaximum(3.0);

    EffectManifestParameterPointer center2 = pManifest->addParameter();
    center2->setId("center2");
    center2->setName(QObject::tr("Center 2"));
    center2->setShortName(QObject::tr("Center 2"));
    center2->setDescription(QObject::tr(
        "Center frequency for Filter 2, from 100 Hz to 14 kHz"));
    center2->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    center2->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    center2->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    center2->setNeutralPointOnScale(0.5);
    center2->setDefault(kDefaultCenter2);
    center2->setMinimum(100);
    center2->setMaximum(14000);

    return pManifest;
}

ParametricEQEffectGroupState::ParametricEQEffectGroupState(
      const mixxx::EngineParameters& bufferParameters)
      : EffectState(bufferParameters) {
    for (int i = 0; i < kBandCount; i++) {
        m_oldGain.append(1.0);
        m_oldQ.append(1.75);
    }

    m_oldCenter.append(kDefaultCenter1);
    m_oldCenter.append(kDefaultCenter2);

    // Initialize the filters with default parameters
    for (int i = 0; i < kBandCount; i++) {
        m_bands.push_back(std::make_unique<EngineFilterBiquad1Peaking>(
                bufferParameters.sampleRate(), m_oldCenter[i], m_oldQ[i]));
    }
}

void ParametricEQEffectGroupState::setFilters(int sampleRate) {
    for (int i = 0; i < kBandCount; i++) {
        m_bands[i]->setFrequencyCorners(
                sampleRate, m_oldCenter[i], m_oldQ[i], m_oldGain[i]);
    }
}

ParametricEQEffect::ParametricEQEffect(EngineEffect* pEffect)
        : m_oldSampleRate(44100) {
    m_pPotGain.append(pEffect->getParameterById("gain1"));
    m_pPotQ.append(pEffect->getParameterById("q1"));
    m_pPotCenter.append(pEffect->getParameterById("center1"));
    m_pPotGain.append(pEffect->getParameterById("gain2"));
    m_pPotQ.append(pEffect->getParameterById("q2"));
    m_pPotCenter.append(pEffect->getParameterById("center2"));
}

ParametricEQEffect::~ParametricEQEffect() {
}

void ParametricEQEffect::processChannel(const ChannelHandle& handle,
                                     ParametricEQEffectGroupState* pState,
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
                    bufferParameters.sampleRate(), fCenter[i], fQ[i], fGain[i]);
        }
    }

    if (fGain[0] != 0) {
        pState->m_bands[0]->process(pInput, pOutput, bufferParameters.samplesPerBuffer());
        if (fGain[1] != 0) {
            pState->m_bands[1]->process(pOutput, pOutput, bufferParameters.samplesPerBuffer());
        } else {
            pState->m_bands[1]->pauseFilter();
        }
    } else {
        pState->m_bands[0]->pauseFilter();
        if (fGain[1] != 0) {
            pState->m_bands[1]->process(pInput, pOutput, bufferParameters.samplesPerBuffer());
        } else {
            pState->m_bands[1]->pauseFilter();
            SampleUtil::copy(pOutput, pInput, bufferParameters.samplesPerBuffer());
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
