#include "effects/native/paneffect.h"

// static
QString PanEffect::getId() {
    return "org.mixxx.effects.pan";
}

// static
EffectManifest PanEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Pan"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Adjust the left/right balance"));

    EffectManifestParameter* left = manifest.addParameter();
    left->setId("left");
    left->setName(QObject::tr("Left"));
    left->setDescription(QObject::tr("Level of the left channel"));
    left->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    left->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    left->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    left->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_RIGHT);
    left->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::INVERTED);
    left->setMinimum(0.0);
    left->setMaximum(1.0);
    left->setDefault(1.0);

    EffectManifestParameter* right = manifest.addParameter();
    right->setId("right");
    right->setName(QObject::tr("Right"));
    right->setDescription(QObject::tr("Level of the right channel"));
    right->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    right->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    right->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    right->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_LEFT);
    right->setMinimum(0.0);
    right->setMaximum(1.0);
    right->setDefault(1.0);

    return manifest;
}

PanEffect::PanEffect(EngineEffect* pEffect, const EffectManifest& manifest)
          : m_pLeftParameter(pEffect->getParameterById("left")),
            m_pRightParameter(pEffect->getParameterById("right")) {
    Q_UNUSED(manifest);
}

PanEffect::~PanEffect() {
}

void PanEffect::processChannel(const ChannelHandle& handle,
                               PanGroupState* pGroupState,
                               const CSAMPLE* pInput,
                               CSAMPLE* pOutput, const unsigned int numSamples,
                               const unsigned int sampleRate,
                               const EffectProcessor::EnableState enableState,
                               const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(pGroupState);
    Q_UNUSED(sampleRate);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);

    CSAMPLE left = m_pLeftParameter->value();
    CSAMPLE right = m_pRightParameter->value();

    for (unsigned int i = 0; i < numSamples; i += 2) {
        pOutput[i] = pInput[i] * left;
        pOutput[i + 1] = pInput[i + 1] * right;
    }
}
