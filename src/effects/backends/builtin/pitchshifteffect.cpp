#include "effects/backends/builtin/pitchshifteffect.h"

#include "util/sample.h"

namespace {
static constexpr SINT kDefaultNorm = 2;
static constexpr SINT kSemitones = 12;
static constexpr SINT kWideRangeOctaves = 2;
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

    EffectManifestParameterPointer rangeMode = pManifest->addParameter();
    rangeMode->setId("rangeMode");
    rangeMode->setName(QObject::tr("Mode"));
    rangeMode->setShortName(QObject::tr("Mode"));
    rangeMode->setDescription(QObject::tr(
            "Set the range mode. There are three possible options "
            "based on the knob position:\n"
            "neutral:\n"
            "\t - knob: default\n"
            "\t - range: from the lowest pitch to the highest pitch\n"
            "positive:\n"
            "\t - knob: maximum\n"
            "\t - range: from the default pitch to the highest pitch\n"
            "negative:\n"
            "\t - knob: minimum\n"
            "\t - range: from the default pitch to the lowest pitch"));
    rangeMode->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    rangeMode->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    rangeMode->setNeutralPointOnScale(0.0);
    rangeMode->setRange(-1.0, 0.0, 1.0);

    EffectManifestParameterPointer semitonesMode = pManifest->addParameter();
    semitonesMode->setId("semitonesMode");
    semitonesMode->setName(QObject::tr("Semitones"));
    semitonesMode->setShortName(QObject::tr("Semitones"));
    semitonesMode->setDescription(QObject::tr(
            "Change pitch in semitone-steps instead of smoothly."));
    semitonesMode->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    semitonesMode->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    semitonesMode->setRange(0, 1, 1);

    EffectManifestParameterPointer wideRange = pManifest->addParameter();
    wideRange->setId("wideRange");
    wideRange->setName(QObject::tr("Wide Range"));
    wideRange->setShortName(QObject::tr("Wide Range"));
    wideRange->setDescription(QObject::tr(
            "Double the pitch scale range."));
    wideRange->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    wideRange->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    wideRange->setRange(0, 0, 1);

    EffectManifestParameterPointer formant = pManifest->addParameter();
    formant->setId("formant");
    formant->setName(QObject::tr("Formant"));
    formant->setShortName(QObject::tr("Formant"));
    formant->setDescription(QObject::tr(
            "Applies the Formant Preserving processing "
            "and activates the handling of the formant shape."));
    formant->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    formant->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    formant->setRange(0, 0, 1);

    return pManifest;
}

void PitchShiftEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPitchParameter = parameters.value("pitch");
    m_pRangeModeParameter = parameters.value("rangeMode");
    m_pSemitonesModeParameter = parameters.value("semitonesMode");
    m_pWideRangeParameter = parameters.value("wideRange");
    m_pFormantParameter = parameters.value("formant");
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

    if (m_currentFormant != m_pFormantParameter->toBool()) {
        m_currentFormant = !m_currentFormant;

        pState->m_pRubberBand->setFormantOption(m_currentFormant
                        ? RubberBand::RubberBandStretcher::
                                  OptionFormantPreserved
                        : RubberBand::RubberBandStretcher::
                                  OptionFormantShifted);
    }

    const double rangeModeParameter = m_pRangeModeParameter->value();

    double pitchParameter = [=, this] {
        if (std::abs(rangeModeParameter) < 0.5) {
            if (m_pWideRangeParameter->toBool()) {
                return m_pPitchParameter->value() * kWideRangeOctaves;
            } else {
                return m_pPitchParameter->value();
            }
        } else {
            if (m_pWideRangeParameter->toBool()) {
                return sgn(rangeModeParameter) * kWideRangeOctaves *
                        (m_pPitchParameter->value() + 1) /
                        kDefaultNorm;
            } else {
                return sgn(rangeModeParameter) *
                        (m_pPitchParameter->value() + 1) /
                        kDefaultNorm;
            }
        }
    }();

    if (m_pSemitonesModeParameter->toBool()) {
        pitchParameter = roundToFraction(pitchParameter, kSemitones);
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
