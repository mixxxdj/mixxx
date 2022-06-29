#include "effects/backends/builtin/pitchshifteffect.h"

#include "util/sample.h"

PitchShiftGroupState::PitchShiftGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters) {
    initializeBuffer(engineParameters);
    audioParametersChanged(engineParameters);
}

PitchShiftGroupState::~PitchShiftGroupState() {
    SampleUtil::free(m_retrieveBuffer[0]);
    SampleUtil::free(m_retrieveBuffer[1]);
}

void PitchShiftGroupState::initializeBuffer(
        const mixxx::EngineParameters& engineParameters) {
    m_retrieveBuffer[0] = SampleUtil::alloc(
            engineParameters.framesPerBuffer());
    m_retrieveBuffer[1] = SampleUtil::alloc(
            engineParameters.framesPerBuffer());
}

void PitchShiftGroupState::audioParametersChanged(
        const mixxx::EngineParameters& engineParameters) {
    m_pRubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
            engineParameters.sampleRate(),
            engineParameters.channelCount(),
            RubberBand::RubberBandStretcher::OptionProcessRealTime);

    m_pRubberBand->setMaxProcessSize(engineParameters.framesPerBuffer());
    m_pRubberBand->setTimeRatio(1.0);
};

// static
QString PitchShiftEffect::getId() {
    return QStringLiteral("org.mixxx.effects.pitchshift");
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
            "The pitch shift applied to the sound."));
    pitch->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pitch->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
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

    const double pitch = 1.0 + [=] {
        if (pitchParameter < 0.0) {
            return pitchParameter / 2.0;
        } else {
            return pitchParameter;
        }
    }();

    pState->m_pRubberBand->setPitchScale(pitch);

    SampleUtil::deinterleaveBuffer(
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            pInput,
            engineParameters.framesPerBuffer());
    pState->m_pRubberBand->process(
            //static_cast<const float* const*>(pState->m_retrieveBuffer),
            pState->m_retrieveBuffer,
            engineParameters.framesPerBuffer(),
            false);

    SINT framesAvailable = pState->m_pRubberBand->available();
    SINT framesToRead = math_min(
            framesAvailable,
            engineParameters.framesPerBuffer());
    SINT receivedFrames = pState->m_pRubberBand->retrieve(
            //static_cast<float* const*>(pState->m_retrieveBuffer),
            pState->m_retrieveBuffer,
            framesToRead);

    SampleUtil::interleaveBuffer(pOutput,
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            receivedFrames);
}
