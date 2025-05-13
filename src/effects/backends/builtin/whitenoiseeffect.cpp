#include "effects/backends/builtin/whitenoiseeffect.h"

#include "control/controlproxy.h"
#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/rampingvalue.h"

namespace {
const QString dryWetParameterId = QStringLiteral("dry_wet");
const QString gainParameterId = QStringLiteral("gain");
} // anonymous namespace

// static
QString WhiteNoiseEffect::getId() {
    return QStringLiteral("org.mixxx.effects.whitenoise");
}

// static
EffectManifestPointer WhiteNoiseEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("White Noise"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr("Mix white noise with the input signal"));
    pManifest->setEffectRampsFromDry(true);

    // This is dry/wet parameter
    EffectManifestParameterPointer drywet = pManifest->addParameter();
    drywet->setId(dryWetParameterId);
    drywet->setName(QObject::tr("Dry/Wet"));
    drywet->setDescription(QObject::tr("Crossfade the noise with the dry signal"));
    drywet->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    drywet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    drywet->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    drywet->setRange(0, 1, 1);

    // This is gain parameter
    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId(gainParameterId);
    gain->setName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("Gain for white noise"));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    gain->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    gain->setRange(0, 1, 1);

    return pManifest;
}

void WhiteNoiseEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDryWetParameter = parameters.value(dryWetParameterId);
    m_pGainParameter = parameters.value(gainParameterId);
}

void WhiteNoiseEffect::processChannel(
        WhiteNoiseGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    WhiteNoiseGroupState& gs = *pState;

    // Get dry/wet control value and set up ramping
    CSAMPLE drywet = static_cast<CSAMPLE>(m_pDryWetParameter->value());
    RampingValue<CSAMPLE_GAIN> drywet_ramping_value(
            drywet, gs.previous_drywet, engineParameters.framesPerBuffer());

    // Generate white noise
    std::uniform_real_distribution<> r_distributor(0.0, 1.0);
    const auto bufferSize = engineParameters.samplesPerBuffer();
    for (unsigned int i = 0; i < bufferSize; ++i) {
        float noise = static_cast<float>(r_distributor(gs.gen));
        // Apply silence threshold to the generated noise
        if (std::abs(noise) < 0.0078f) {
            noise = 0.0f;
        }
        gs.m_noiseBuffer[i] = noise;
    }

    // Apply high-pass and low-pass filtering to the noise
    gs.m_highpass.process(gs.m_noiseBuffer.data(), gs.m_filteredBuffer.data(), bufferSize);
    gs.m_lowpass.process(gs.m_filteredBuffer.data(), gs.m_filteredBuffer.data(), bufferSize);

    // Get the master gain value
    CSAMPLE gain = static_cast<CSAMPLE>(m_pGainParameter->value());

    // Mix dry and wet signals, apply gain and ramp the dry/wet effect
    for (unsigned int i = 0; i < bufferSize; ++i) {
        CSAMPLE_GAIN drywet_ramped = drywet_ramping_value.getNth(i);

        // Apply the dry/wet control and gain to the output signal
        pOutput[i] = (pInput[i] * (1 - drywet_ramped) +
                             gs.m_filteredBuffer[i] * drywet_ramped) *
                gain;
    }

    // Store the current drywet value for the next buffer
    if (enableState == EffectEnableState::Disabling) {
        gs.previous_drywet = 0;
    } else {
        gs.previous_drywet = drywet;
    }
}
