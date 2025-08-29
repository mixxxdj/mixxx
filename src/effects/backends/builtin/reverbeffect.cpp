#include "effects/backends/builtin/reverbeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

// static
QString ReverbEffect::getId() {
    return "org.mixxx.effects.reverb";
}

// static
EffectManifestPointer ReverbEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setAddDryToWet(true);
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
    decay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    decay->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    decay->setRange(0, 0.5, 1);

    EffectManifestParameterPointer bandwidth = pManifest->addParameter();
    bandwidth->setId("bandwidth");
    bandwidth->setName(QObject::tr("Bandwidth"));
    bandwidth->setShortName(QObject::tr("BW"));
    bandwidth->setDescription(QObject::tr(
            "Bandwidth of the low pass filter at the input.\n"
            "Higher values result in less attenuation of high frequencies."));
    bandwidth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bandwidth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bandwidth->setRange(0, 1, 1);

    EffectManifestParameterPointer damping = pManifest->addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("Damping"));
    damping->setShortName(QObject::tr("Damping"));
    damping->setDescription(
            QObject::tr("Higher damping values cause high frequencies to decay "
                        "more quickly than low frequencies."));
    damping->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    damping->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    damping->setRange(0, 0, 1);

    EffectManifestParameterPointer send = pManifest->addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setShortName(QObject::tr("Send"));
    send->setDescription(QObject::tr(
            "How much of the signal to send in to the effect"));
    send->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    send->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::NotInverted);
    send->setRange(0, 0, 1);

    return pManifest;
}

void ReverbEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDecayParameter = parameters.value("decay");
    m_pBandWidthParameter = parameters.value("bandwidth");
    m_pDampingParameter = parameters.value("damping");
    m_pSendParameter = parameters.value("send_amount");
}

void ReverbEffect::processChannel(
        ReverbGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    const auto decay = static_cast<sample_t>(m_pDecayParameter->value());
    const auto bandwidth = static_cast<sample_t>(m_pBandWidthParameter->value());
    const auto damping = static_cast<sample_t>(m_pDampingParameter->value());
    const auto sendCurrent = static_cast<sample_t>(m_pSendParameter->value());

    if (pState->sampleRate != engineParameters.sampleRate()) {
        pState->reverb.setSamplerate(engineParameters.sampleRate());
        pState->sampleRate = engineParameters.sampleRate();
    }

    // Reinitialize the effect when turning it on to prevent replaying the old buffer
    // from the last time the effect was enabled..
    if (enableState == EffectEnableState::Enabling) {
        pState->reverb.activate();
    }

    pState->reverb.processBuffer(pInput,
            pOutput,
            engineParameters.samplesPerBuffer(),
            bandwidth,
            decay,
            damping,
            sendCurrent,
            pState->sendPrevious);

    // The ramping of the send parameter handles ramping when enabling, so
    // this effect must handle ramping to dry when disabling itself (instead
    // of being handled by EngineEffect::process).
    if (enableState == EffectEnableState::Disabling) {
        SampleUtil::applyRampingGain(pOutput, 1.0, 0.0, engineParameters.samplesPerBuffer());
        pState->sendPrevious = 0;
    } else {
        pState->sendPrevious = sendCurrent;
    }
}
