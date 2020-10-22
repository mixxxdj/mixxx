#include "effects/builtin/whitenoiseeffect.h"

#include "util/rampingvalue.h"

namespace {
const QString dryWetParameterId = QStringLiteral("dry_wet");
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
    EffectManifestParameterPointer intensity = pManifest->addParameter();
    intensity->setId(dryWetParameterId);
    intensity->setName(QObject::tr("Dry/Wet"));
    intensity->setDescription(QObject::tr("Crossfade the noise with the dry signal"));
    intensity->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    intensity->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    intensity->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    intensity->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    intensity->setMinimum(0);
    intensity->setDefault(0);
    intensity->setMaximum(1);

    return pManifest;
}

WhiteNoiseEffect::WhiteNoiseEffect(EngineEffect* pEffect)
        : m_pDryWetParameter(pEffect->getParameterById(dryWetParameterId)) {
}

WhiteNoiseEffect::~WhiteNoiseEffect() {
}

void WhiteNoiseEffect::processChannel(
        const ChannelHandle& handle,
        WhiteNoiseGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    WhiteNoiseGroupState& gs = *pGroupState;

    CSAMPLE drywet = static_cast<CSAMPLE>(m_pDryWetParameter->value());
    RampingValue<CSAMPLE_GAIN> drywet_ramping_value(
            drywet, gs.previous_drywet, bufferParameters.framesPerBuffer());

    std::uniform_real_distribution<> r_distributor(0.0, 1.0);

    for (unsigned int i = 0; i < bufferParameters.samplesPerBuffer(); i++) {
        CSAMPLE_GAIN drywet_ramped = drywet_ramping_value.getNext();

        float noise = static_cast<float>(
                r_distributor(gs.gen));

        pOutput[i] = pInput[i] * (1 - drywet_ramped) + noise * drywet_ramped;
    }

    if (enableState == EffectEnableState::Disabling) {
        gs.previous_drywet = 0;
    } else {
        gs.previous_drywet = drywet;
    }
}
