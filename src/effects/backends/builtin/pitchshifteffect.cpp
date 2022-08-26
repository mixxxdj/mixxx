#include "effects/backends/builtin/pitchshifteffect.h"

#include "util/sample.h"

namespace {
static constexpr SINT kSemitonesPerOctave = 12;
} // anonymous namespace

PitchShiftEffect::PitchShiftEffect()
        : m_currentFormant(false) {
}

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

    EffectManifestParameterPointer range = pManifest->addParameter();
    range->setId("range");
    range->setName(QObject::tr("Range"));
    range->setShortName(QObject::tr("Range"));
    range->setDescription(QObject::tr(
            "The range of the Pitch knob (0 - 2 octaves).\n"));
    range->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    range->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    range->setNeutralPointOnScale(1.0);
    range->setRange(0.0, 1.0, 2.0);

    EffectManifestParameterPointer semitonesMode = pManifest->addParameter();
    semitonesMode->setId("semitonesMode");
    semitonesMode->setName(QObject::tr("Semitones"));
    semitonesMode->setShortName(QObject::tr("Semitones"));
    semitonesMode->setDescription(QObject::tr(
            "Change the pitch in semitone steps instead of continuously."));
    semitonesMode->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    semitonesMode->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    semitonesMode->setRange(0, 1, 1);

    EffectManifestParameterPointer formantPreserving = pManifest->addParameter();
    formantPreserving->setId("formantPreserving");
    formantPreserving->setName(QObject::tr("Formant"));
    formantPreserving->setShortName(QObject::tr("Formant"));
    formantPreserving->setDescription(QObject::tr(
            "Preserve the resonant frequencies (formants) of the human vocal tract "
            "and other instruments.\n"
            "Hint: compensates \"chipmunk\" or \"growling\" voices"));
    formantPreserving->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    formantPreserving->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    formantPreserving->setRange(0, 0, 1);

    return pManifest;
}

void PitchShiftEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPitchParameter = parameters.value("pitch");
    m_pRangeParameter = parameters.value("range");
    m_pSemitonesModeParameter = parameters.value("semitonesMode");
    m_pFormantPreservingParameter = parameters.value("formantPreserving");
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

    if (const bool formantPreserving = m_pFormantPreservingParameter->toBool();
            m_currentFormant != formantPreserving) {
        m_currentFormant = formantPreserving;

        pState->m_pRubberBand->setFormantOption(m_currentFormant
                        ? RubberBand::RubberBandStretcher::
                                  OptionFormantPreserved
                        : RubberBand::RubberBandStretcher::
                                  OptionFormantShifted);
    }

    double pitchParameter = m_pPitchParameter->value() * m_pRangeParameter->value();

    if (m_pSemitonesModeParameter->toBool()) {
        pitchParameter = roundToFraction(pitchParameter, kSemitonesPerOctave);
    }

    const double pitch = std::pow(2.0, pitchParameter);

    pState->m_pRubberBand->setPitchScale(pitch);

    SampleUtil::deinterleaveBuffer(
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            pInput,
            engineParameters.framesPerBuffer());
    pState->m_pRubberBand->process(
            pState->m_retrieveBuffer,
            engineParameters.framesPerBuffer(),
            false);

    SINT framesAvailable = pState->m_pRubberBand->available();
    SINT framesToRead = math_min(
            framesAvailable,
            engineParameters.framesPerBuffer());
    SINT receivedFrames = pState->m_pRubberBand->retrieve(
            pState->m_retrieveBuffer,
            framesToRead);

    SampleUtil::interleaveBuffer(pOutput,
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            receivedFrames);
}
