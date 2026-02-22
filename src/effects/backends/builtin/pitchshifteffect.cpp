#include "effects/backends/builtin/pitchshifteffect.h"

#include <rubberband/RubberBandStretcher.h>

#include <QString>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

namespace {
static constexpr SINT kSemitonesPerOctave = 12;

static const QString kPitchParameterId = QStringLiteral("pitch");
static const QString kRangeParameterId = QStringLiteral("range");
static const QString kSemitonesModeParameterId = QStringLiteral("semitonesMode");
static const QString kFormantPreservingParameterId = QStringLiteral("formantPreserving");
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
}

void PitchShiftGroupState::initializeBuffer(
        const mixxx::EngineParameters& engineParameters) {
    m_retrieveBuffer[0] = mixxx::SampleBuffer(
            engineParameters.framesPerBuffer());
    m_retrieveBuffer[1] = mixxx::SampleBuffer(
            engineParameters.framesPerBuffer());
}

void PitchShiftGroupState::audioParametersChanged(
        const mixxx::EngineParameters& engineParameters) {
    m_pRubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
            engineParameters.sampleRate(),
            engineParameters.channelCount(),
            RubberBand::RubberBandStretcher::OptionProcessRealTime);

    // Set the maximum number of frames that will be ever passing into a single
    // RubberBand::RubberBandStretcher::process call.
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
    pManifest->setVersion("2.0");
    pManifest->setDescription(QObject::tr(
            "Raises or lowers the original pitch of a sound."));
    pManifest->setMetaknobDefault(0.5);

    EffectManifestParameterPointer pitch = pManifest->addParameter();
    pitch->setId(kPitchParameterId);
    pitch->setName(QObject::tr("Pitch"));
    pitch->setShortName(QObject::tr("Pitch"));
    pitch->setDescription(QObject::tr(
            "The pitch shift applied to the sound."));
    pitch->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pitch->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pitch->setNeutralPointOnScale(0.5);
    pitch->setRange(-1.0, 0.0, 1.0);

    EffectManifestParameterPointer range = pManifest->addParameter();
    range->setId(kRangeParameterId);
    range->setName(QObject::tr("Range"));
    range->setShortName(QObject::tr("Range"));
    range->setDescription(QObject::tr(
            "The range of the Pitch knob (0 - 2 octaves).\n"));
    range->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    range->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    range->setNeutralPointOnScale(0.5);
    range->setRange(0.0, 1.0, 2.0);

    EffectManifestParameterPointer semitonesMode = pManifest->addParameter();
    semitonesMode->setId(kSemitonesModeParameterId);
    semitonesMode->setName(QObject::tr("Semitones"));
    semitonesMode->setShortName(QObject::tr("Semitones"));
    semitonesMode->setDescription(QObject::tr(
            "Change the pitch in semitone steps instead of continuously."));
    semitonesMode->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    semitonesMode->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    semitonesMode->setRange(0, 1, 1);

    EffectManifestParameterPointer formantPreserving = pManifest->addParameter();
    formantPreserving->setId(kFormantPreservingParameterId);
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
    m_pPitchParameter = parameters.value(kPitchParameterId);
    m_pRangeParameter = parameters.value(kRangeParameterId);
    m_pSemitonesModeParameter = parameters.value(kSemitonesModeParameterId);
    m_pFormantPreservingParameter = parameters.value(kFormantPreservingParameterId);
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

    DEBUG_ASSERT(engineParameters.framesPerBuffer() <= pState->m_retrieveBuffer[0].size());

    if (const bool formantPreserving = m_pFormantPreservingParameter->toBool();
            m_currentFormant != formantPreserving) {
        m_currentFormant = formantPreserving;

        pState->m_pRubberBand->setFormantOption(m_currentFormant
                        ? RubberBand::RubberBandStretcher::
                                  OptionFormantPreserved
                        : RubberBand::RubberBandStretcher::
                                  OptionFormantShifted);
    }

    // The range of the scale of the Pitch parameter is <-1.0, 1.0>
    // with the middle position 0.0. On the other hand, the range
    // of the scale of the Range parameter is <0.0, 2.0> with the middle
    // position 1.0. With that, the resulting pitch is obtained using the Pitch
    // and Range settings with the following implications for some examples:
    //
    // With the default range set to 1.0, the resulting pitch value stored
    // in the pitchParameter is in the range <0.5, 2.0>, so the pitch
    // can be changed from one octave down to one octave up.
    //
    // 1. Range (default): 1.0
    //    - Pitch (default):  0.0 => pow(2.0,  0.0) = 1.0  => pitch is unchanged
    //    - Pitch:           -1.0 => pow(2.0, -1.0) = 0.5  => one octave down
    //    - Pitch:            1.0 => pow(2.0,  1.0) = 2.0  => one octave up
    //
    // With the default range set to 2.0, the resulting pitch value stored
    // in the pitchParameter is in range <0.25, 4.0>, so the pitch
    // can be changed from two octaves down to two octaves up.
    //
    // 2. Range: 2.0
    //    - Pitch (default):  0.0 => pow(2.0,  0.0) = 1.0  => pitch is unchanged
    //    - Pitch:           -1.0 => pow(2.0, -2.0) = 0.25 => two octaves down
    //    - Pitch:            1.0 => pow(2.0,  2.0) = 4.0  => two octaves up
    //
    // When the range is set to 0.0, the resulting pitch value that is stored
    // in the pitchParameter is exactly 1.0 only, so the pitch does not change.
    //
    // 3. Range: 0.0
    //    - Pitch (default):  0.0 => pow(2.0,  0.0) = 1.0  => pitch is unchanged
    //    - Pitch:           -1.0 => pow(2.0,  0.0) = 1.0  => pitch is unchanged
    //    - Pitch:            1.0 => pow(2.0,  0.0) = 1.0  => pitch is unchanged
    //
    // The reason for keeping this range value is that if the pitch value is fixed
    // by changing the range value, the resulting pitch can only change in one way (up or down)
    // and with a specific scale range based on the pitch fixed setting.
    //
    // 1. Pitch: 1.0
    //    - Range: 0.0 => pow(2.0, 0.0) = 1.0 => pitch is unchanged
    //    - Range: 1.0 => pow(2.0, 1.0) = 2.0 => one octave up
    //    - Range: 2.0 => pow(2.0, 2.0) = 4.0 => two octaves up
    //
    // 2. Pitch: -1.0
    //    - Range: 0.0 => pow(2.0,  0.0) = 1.0  => pitch is unchanged
    //    - Range: 1.0 => pow(2.0, -1.0) = 0.5  => one octave down
    //    - Range: 2.0 => pow(2.0, -2.0) = 0.25 => two octaves down
    double pitchParameter = m_pPitchParameter->value() * m_pRangeParameter->value();

    // Choose the scale of the Pitch knob, and based on that, recalculate the pitch value.
    // There are 2 possible scales:
    // 1. Semitones mode (true) - The Pitch knob parameter value is recalculated
    //    to proceed on the semitone chromatic scale. By default,
    //    this mode is used.
    // 2. Continuous mode (false) - The Pitch knob changes values continuously,
    //    the same as the RubberBand classic approach.
    if (m_pSemitonesModeParameter->toBool()) {
        // To keep the values in the semitone chromatic scale, the pitch value
        // must be rounded to a fraction of the scale in one octave. So,
        // the value has to be rounded to the fraction of the number
        // of semitones in octave.
        pitchParameter = roundToFraction(pitchParameter, kSemitonesPerOctave);
    }

    // In equal temperament, all the semitones have the same size (100 cents),
    // and there are twelve semitones in an octave (1200 cents). As a result,
    // the notes of an equal-tempered chromatic scale are equally-spaced.
    // The pitch scaling ratio corresponds to a shift of S equal-tempered
    // semitones (where S is positive for an upwards shift and negative
    // for downwards) is pow(2.0, S / 12.0). The pitch scaling ratio
    // is the ratio of target frequency to source frequency. For example,
    // a ratio of 2.0 would shift up by one octave, 0.5 down by one octave,
    // or 1.0 leaving the pitch unaffected.
    const double pitch = std::pow(2.0, pitchParameter);

    pState->m_pRubberBand->setPitchScale(pitch);

    SampleUtil::deinterleaveBuffer(
            pState->m_retrieveBuffer[0].data(),
            pState->m_retrieveBuffer[1].data(),
            pInput,
            engineParameters.framesPerBuffer());

    CSAMPLE* retrieveBuffers[2]{pState->m_retrieveBuffer[0].data(),
            pState->m_retrieveBuffer[1].data()};
    pState->m_pRubberBand->process(
            retrieveBuffers,
            engineParameters.framesPerBuffer(),
            false);

    SINT framesAvailable = pState->m_pRubberBand->available();
    SINT framesToRead = math_min(
            framesAvailable,
            engineParameters.framesPerBuffer());

    SINT receivedFrames = pState->m_pRubberBand->retrieve(
            retrieveBuffers,
            framesToRead);

    SampleUtil::interleaveBuffer(pOutput,
            pState->m_retrieveBuffer[0].data(),
            pState->m_retrieveBuffer[1].data(),
            receivedFrames);
}
