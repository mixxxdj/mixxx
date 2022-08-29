#include "effects/backends/builtin/pitchshifteffect.h"

#include <QString>

#include "util/counter.h"
#include "util/math.h"
#include "util/sample.h"

namespace {
// TODO for the setting of sample rate 44100Hz, input/output buffer size
//  of 1024 frames (2048 samples) can be this value probably just 4096.
static constexpr SINT kReservedFrames = 8192;
static constexpr SINT kSemitonesPerOctave = 12;

static const QString kPitchParameterId = QStringLiteral("pitch");
static const QString kRangeParameterId = QStringLiteral("range");
static const QString kSemitonesModeParameterId = QStringLiteral("semitonesMode");
static const QString kFormantPreservingParameterId = QStringLiteral("formantPreserving");
} // anonymous namespace

PitchShiftEffect::PitchShiftEffect()
        : m_currentFormant(false),
          m_fullRingBuffer(false),
          m_groupDelayFrames(0) {
}

PitchShiftGroupState::PitchShiftGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters) {
    //audioParametersChanged(engineParameters);
    initializeBuffer(engineParameters);
}

PitchShiftGroupState::~PitchShiftGroupState() {
    SampleUtil::free(m_retrieveBuffer[0]);
    SampleUtil::free(m_retrieveBuffer[1]);
    SampleUtil::free(m_interleavedBuffer);
}

void PitchShiftGroupState::initializeBuffer(
        const mixxx::EngineParameters& engineParameters) {
    m_inputBuffer = std::make_unique<CircularBuffer<CSAMPLE>>(
            kReservedFrames * engineParameters.channelCount());
    m_interleavedBuffer = SampleUtil::alloc(
            kReservedFrames * engineParameters.channelCount());

    m_retrieveBuffer[0] = SampleUtil::alloc(kReservedFrames);
    m_retrieveBuffer[1] = SampleUtil::alloc(kReservedFrames);
}

// TODO(davidchocholaty) there is needed to fix the bug in the Mixxx
//  as first because the function caller will pass the invalid value of
//  engineParameters (just the maximum possible values).

//void PitchShiftGroupState::audioParametersChanged(
//        const mixxx::EngineParameters& engineParameters) {
//    m_pRubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
//            engineParameters.sampleRate(),
//            engineParameters.channelCount(),
//            RubberBand::RubberBandStretcher::OptionProcessRealTime |
//                    RubberBand::RubberBandStretcher::OptionPitchHighConsistency);
//}

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

    EffectManifestParameterPointer pitch = pManifest->addParameter();
    pitch->setId(kPitchParameterId);
    pitch->setName(QObject::tr("Pitch"));
    pitch->setShortName(QObject::tr("Pitch"));
    pitch->setDescription(QObject::tr(
            "The pitch shift applied to the sound."));
    pitch->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pitch->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pitch->setNeutralPointOnScale(0.0);
    pitch->setRange(-1.0, 0.0, 1.0);

    EffectManifestParameterPointer range = pManifest->addParameter();
    range->setId(kRangeParameterId);
    range->setName(QObject::tr("Range"));
    range->setShortName(QObject::tr("Range"));
    range->setDescription(QObject::tr(
            "The range of the Pitch knob (0 - 2 octaves).\n"));
    range->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    range->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    range->setNeutralPointOnScale(1.0);
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

    if (!pState->m_pRubberBand) {
        pState->m_pRubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
                engineParameters.sampleRate(),
                engineParameters.channelCount(),
                RubberBand::RubberBandStretcher::OptionProcessRealTime |
                        RubberBand::RubberBandStretcher::OptionPitchHighConsistency);
    }

    //if (m_groupDelayFrames == 0) {
    // TODO(davidchocholaty) verify if the kReservedFrames is divisible
    //  by the number of frames, so we can just subtract the last buffer
    //  from the group delay frames.
    //    m_groupDelayFrames = kReservedFrames - engineParameters.framesPerBuffer();
    //}

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

    // TODO(davidchocholaty) handle the situation if it would not be possible
    //  to write the input buffer samples into the input ring buffer due to
    //  it being full. This situation means, that the input is produced
    //  much faster than the RubberBand takes the samples from the ring buffer.
    pState->m_inputBuffer->write(pInput, engineParameters.samplesPerBuffer());

    if (pState->m_inputBuffer->isFull()) {
        m_fullRingBuffer = true;
    }

    if (!m_fullRingBuffer) {
        SampleUtil::fill(pOutput, 0.0f, engineParameters.samplesPerBuffer());
        return;
    }

    bool lastReadFailed = false;
    bool inputDataMiss = false;

    SINT remainingFrames = engineParameters.framesPerBuffer();
    SINT retrievedFrames = 0;

    pState->m_offset[0] = pState->m_retrieveBuffer[0];
    pState->m_offset[1] = pState->m_retrieveBuffer[1];

    while (remainingFrames > 0 && !inputDataMiss) {
        pState->m_offset[0] += retrievedFrames;
        pState->m_offset[1] += retrievedFrames;

        SINT requiredFrames = static_cast<SINT>(pState->m_pRubberBand->getSamplesRequired());

        SINT readFrames = pState->m_inputBuffer->read(
                                  pState->m_interleavedBuffer,
                                  requiredFrames * engineParameters.channelCount()) /
                engineParameters.channelCount();

        if (readFrames >= requiredFrames) {
            lastReadFailed = false;

            SampleUtil::deinterleaveBuffer(
                    pState->m_offset[0],
                    pState->m_offset[1],
                    pState->m_interleavedBuffer,
                    requiredFrames);

            pState->m_pRubberBand->process(
                    pState->m_offset,
                    requiredFrames,
                    false);
        } else {
            if (lastReadFailed) {
                // Flush and break out after the next retrieval. If we are
                // at EOF this serves to get the last samples out of
                // RubberBand.
                pState->m_pRubberBand->process(
                        pState->m_offset,
                        0,
                        true);

                inputDataMiss = true;
            }

            lastReadFailed = true;
        }

        SINT availableFrames = pState->m_pRubberBand->available();
        SINT toReadFrames = math_min(availableFrames, remainingFrames);

        retrievedFrames = static_cast<SINT>(pState->m_pRubberBand->retrieve(
                pState->m_offset, toReadFrames));

        remainingFrames -= retrievedFrames;
    }

    if (inputDataMiss) {
        const SINT writtenFrames = engineParameters.framesPerBuffer() - remainingFrames;

        SampleUtil::interleaveBuffer(pOutput,
                pState->m_retrieveBuffer[0],
                pState->m_retrieveBuffer[1],
                writtenFrames);

        SampleUtil::fill(
                pOutput + writtenFrames * engineParameters.channelCount(),
                0.0f,
                remainingFrames);

        pState->m_pRubberBand->reset();

        Counter counter("PitchShiftEffect::processChannel underflow");
        counter.increment();
    } else {
        SampleUtil::interleaveBuffer(pOutput,
                pState->m_retrieveBuffer[0],
                pState->m_retrieveBuffer[1],
                engineParameters.framesPerBuffer());
    }
}
