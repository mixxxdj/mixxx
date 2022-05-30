#include "effects/backends/builtin/pitchshifteffect.h"

#include "util/sample.h"

PitchShiftGroupState::~PitchShiftGroupState() noexcept {
    SampleUtil::free(m_retrieveBuffer[0]);
    SampleUtil::free(m_retrieveBuffer[1]);

    VERIFY_OR_DEBUG_ASSERT(m_pRubberBand) {
        return;
    }

    m_pRubberBand->reset();
}

// static
QString PitchShiftEffect::getId() {
    return "org.mixxx.effects.pitchshift";
}

//static
EffectManifestPointer PitchShiftEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Pitch Shift"));
    pManifest->setShortName(QObject::tr("Pitch Shift"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Raises or lowers the original pitch of a sound."));

    EffectManifestParameterPointer pitch = pManifest->addParameter();
    pitch->setId("pitch");
    pitch->setName(QObject::tr("Pitch"));
    pitch->setShortName(QObject::tr("Pitch"));
    pitch->setDescription(QObject::tr(
            "The new pitch of a sound."));
    pitch->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pitch->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pitch->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pitch->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::Inverted);
    pitch->setNeutralPointOnScale(0.0);
    pitch->setRange(-1.0, 0.0, 1.0);

    return pManifest;
}

void PitchShiftEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPitchParameter = parameters.value("pitch");
}

void PitchShiftEffect::processChannel(
        PitchShiftGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(enableState);

    const double pitchParameter = m_pPitchParameter->value();

    double pitch = 1.0;

    if (pitchParameter < 0.0) {
        pitch += pitchParameter / 2.0;
    } else {
        pitch += pitchParameter;
    }

    pState->m_pRubberBand->setPitchScale(pitch);

    SampleUtil::deinterleaveBuffer(
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            pInput,
            engineParameters.framesPerBuffer());
    pState->m_pRubberBand->process(
            (const float* const*)pState->m_retrieveBuffer,
            engineParameters.framesPerBuffer(),
            false);

    SINT framesAvailable = pState->m_pRubberBand->available();
    SINT framesToRead = math_min(
            framesAvailable,
            engineParameters.framesPerBuffer());
    SINT receivedFrames = pState->m_pRubberBand->retrieve(
            (float* const*)pState->m_retrieveBuffer,
            framesToRead);

    SampleUtil::interleaveBuffer(pOutput,
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            receivedFrames);
}
