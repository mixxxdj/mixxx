#include "effects/builtin/reverbeffect.h"

#include <QtDebug>

#include "util/sample.h"

// static
QString ReverbEffect::getId() {
    return "org.mixxx.effects.reverb";
}

// static
EffectManifestPointer ReverbEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    // Refer to EngineEffectChain::process for how this manifest attribute is
    // handled.
    pManifest->setAddDryToWet(true);
    // EngineEffect::process cannot appropriately ramp this effect on because
    // the effect does not start outputting sound immediately. Instead,
    // ramping on is handled by ramping the Send parameter from 0.
    pManifest->setEffectRampsFromDry(true);

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Reverb"));
    pManifest->setAuthor("The Mixxx Team, CAPS Plugins");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
        "Emulates the sound of the signal bouncing off the walls of a room"));

    EffectManifestParameterPointer decay = pManifest->addParameter();
    decay->setId("decay");
    decay->setName(QObject::tr("Decay"));
    decay->setShortName(QObject::tr("Decay"));
    decay->setDescription(QObject::tr(
        "Lower decay values cause reverberations to fade out more quickly."));
    decay->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    decay->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    decay->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    decay->setMinimum(0);
    decay->setDefault(0.5);
    decay->setMaximum(1);

    EffectManifestParameterPointer bandwidth = pManifest->addParameter();
    bandwidth->setId("bandwidth");
    bandwidth->setName(QObject::tr("Bandwidth"));
    bandwidth->setShortName(QObject::tr("BW"));
    bandwidth->setDescription(QObject::tr(
        "Bandwidth of the low pass filter at the input.\n"
        "Higher values result in less attenuation of high frequencies."));
    bandwidth->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    bandwidth->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    bandwidth->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    bandwidth->setMinimum(0);
    bandwidth->setDefault(1);
    bandwidth->setMaximum(1);

    EffectManifestParameterPointer damping = pManifest->addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("Damping"));
    damping->setShortName(QObject::tr("Damping"));
    damping->setDescription(QObject::tr(
      "Higher damping values cause high frequencies to decay more quickly than low frequencies."));
    damping->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    damping->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    damping->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    damping->setMinimum(0);
    damping->setDefault(0);
    damping->setMaximum(1);

    EffectManifestParameterPointer send = pManifest->addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setShortName(QObject::tr("Send"));
    send->setDescription(QObject::tr(
        "How much of the signal to send in to the effect"));
    send->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    send->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    send->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::NOT_INVERTED);
    send->setMinimum(0);
    send->setDefault(0);
    send->setMaximum(1);

    return pManifest;
}

ReverbEffect::ReverbEffect(EngineEffect* pEffect)
        : m_pDecayParameter(pEffect->getParameterById("decay")),
          m_pBandWidthParameter(pEffect->getParameterById("bandwidth")),
          m_pDampingParameter(pEffect->getParameterById("damping")),
          m_pSendParameter(pEffect->getParameterById("send_amount")) {
}

ReverbEffect::~ReverbEffect() {
    //qDebug() << debugString() << "destroyed";
}

void ReverbEffect::processChannel(const ChannelHandle& handle,
                                ReverbGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const mixxx::EngineParameters& bufferParameters,
                                const EffectEnableState enableState,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    const auto decay = m_pDecayParameter->value();
    const auto bandwidth = m_pBandWidthParameter->value();
    const auto damping = m_pDampingParameter->value();
    pState->send.setCurrentCallbackValue(pow(m_pSendParameter->value(), 1.53));

    // Reinitialize the effect when turning it on to prevent replaying the old buffer
    // from the last time the effect was enabled.
    // Also, update the sample rate if it has changed.
    if (enableState == EffectEnableState::Enabling
        || pState->sampleRate != bufferParameters.sampleRate()) {
        pState->reverb.init(bufferParameters.sampleRate());
        pState->engineParametersChanged(bufferParameters);
    }

    pState->reverb.processBuffer(pInput, pOutput,
                                 bufferParameters.samplesPerBuffer(),
                                 bandwidth, decay, damping, &pState->send);

    // The ramping of the send parameter handles ramping when enabling, so
    // this effect must handle ramping to dry when disabling itself (instead
    // of being handled by EngineEffect::process).
    if (enableState == EffectEnableState::Disabling) {
        SampleUtil::applyRampingGain(pOutput, 1.0, 0.0, bufferParameters.samplesPerBuffer());
        pState->send.setCurrentCallbackValue(0);
    }
}
