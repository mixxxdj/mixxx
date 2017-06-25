#include "balanceeffect.h"

namespace {

} // anonymous namespace

// static
QString BalanceEffect::getId() {
    return "org.mixxx.effects.balance";
}

// static
EffectManifest BalanceEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Balance"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Adjust the left/right balance and Stereo width"));

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

    EffectManifestParameter* midSide = manifest.addParameter();
    midSide->setId("midSide");
    midSide->setName(QObject::tr("Mid/Side"));
    midSide->setDescription(QObject::tr("Balance of Mid and Side"));
    midSide->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    midSide->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    midSide->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    midSide->setDefaultLinkType(EffectManifestParameter::LinkType::NONE);
    midSide->setMinimum(0);
    midSide->setMaximum(1.0);
    midSide->setDefault(0.5);

    return manifest;
}

BalanceEffect::BalanceEffect(EngineEffect* pEffect, const EffectManifest& manifest)
          : m_pLeftParameter(pEffect->getParameterById("left")),
            m_pRightParameter(pEffect->getParameterById("right")),
            m_pMidSideParameter(pEffect->getParameterById("midSide")) {
    Q_UNUSED(manifest);
}

BalanceEffect::~BalanceEffect() {
}

void BalanceEffect::processChannel(const ChannelHandle& handle,
                               BalanceGroupState* pGroupState,
                               const CSAMPLE* pInput,
                               CSAMPLE* pOutput, const unsigned int numSamples,
                               const unsigned int sampleRate,
                               const EffectProcessor::EnableState enableState,
                               const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(sampleRate);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);

    CSAMPLE_GAIN left = m_pLeftParameter->value();
    CSAMPLE_GAIN right = m_pRightParameter->value();
    CSAMPLE_GAIN midSide = m_pMidSideParameter->value();

    CSAMPLE_GAIN leftDelta = (left - pGroupState->oldLeft)
                    / CSAMPLE_GAIN(numSamples / 2);
    CSAMPLE_GAIN rightDelta = (right - pGroupState->oldRight)
                    / CSAMPLE_GAIN(numSamples / 2);
    CSAMPLE_GAIN midSideDelta = (midSide - pGroupState->oldMidSide)
                    / CSAMPLE_GAIN(numSamples / 2);

    CSAMPLE_GAIN leftStart = left - leftDelta;
    CSAMPLE_GAIN rightStart = right - rightDelta;
    CSAMPLE_GAIN midSideStart = midSide - midSideDelta;

    for (SINT i = 0; i < numSamples / 2; ++i) {
        CSAMPLE mid = (pInput[i * 2]  + pInput[i * 2 + 1]) / 2.0f;
        CSAMPLE side = (pInput[i * 2 + 1] - pInput[i * 2]) / 2.0f;
        if (midSide > 0.5) {
            mid *= 2 * (1 - (midSideStart + midSideDelta * i));
        } else {
            side *= 2 * (midSideStart + midSideDelta * i);
        }
        pOutput[i * 2] = (mid - side) * (leftStart + leftDelta * i);
        pOutput[i * 2 + 1] = (mid + side) * (rightStart + rightDelta * i);
    }

    pGroupState->oldLeft = left;
    pGroupState->oldRight = right;
    pGroupState->oldMidSide = midSide;
}
