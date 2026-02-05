#include "effects/backends/builtin/gaineffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/math.h"
#include "util/rampingvalue.h"

namespace {
inline CSAMPLE tanh_approx(CSAMPLE input) {
    return input / (1 + input * input / (3 + input * input / 5));
}
} // namespace

// static
QString GainEffect::getId() {
    return "org.mixxx.effects.gain";
}

// static
EffectManifestPointer GainEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Gain"));
    pManifest->setShortName(QObject::tr("Gain"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr("Amplifies audio by gain amount"));
    pManifest->setEffectRampsFromDry(true);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Gain"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("The gain of the samples"));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    gain->setNeutralPointOnScale(0.0);
    gain->setRange(-24.0, 0.0, 24.0);

    EffectManifestParameterPointer clip = pManifest->addParameter();
    clip->setId("clip");
    clip->setName(QObject::tr("Clip"));
    clip->setShortName(QObject::tr("Clip"));
    clip->setDescription(QObject::tr("Clip samples to within -1 to 1"));
    clip->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    clip->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    clip->setRange(0, 0, 1);

    return pManifest;
}

void GainEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pGainParameter = parameters.value("gain");
    m_pClipParameter = parameters.value("clip");
}

void GainEffect::processChannel(
        GainGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(enableState);

    const CSAMPLE_GAIN gain = db2ratio(static_cast<float>(m_pGainParameter->value()));
    const auto clip = m_pClipParameter->toBool();
    GainGroupState& gs = *pState;
    RampingValue<CSAMPLE_GAIN> gain_ramping_value(
            gs.previous_gain, gain, engineParameters.samplesPerBuffer());

    for (SINT i = 0;
            i < engineParameters.samplesPerBuffer();
            i++) {
        const auto gain_ramped = gain_ramping_value.getNth(i);
        pOutput[i] = pInput[i] * gain_ramped;

        if (clip) {
            pOutput[i] = tanh_approx(pOutput[i]);
        }
    }

    if (enableState == EffectEnableState::Disabling) {
        gs.previous_gain = 0;
    } else {
        gs.previous_gain = gain;
    }
}
