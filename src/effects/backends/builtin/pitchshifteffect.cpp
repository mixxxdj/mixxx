#include "effects/backends/builtin/pitchshifteffect.h"

#include "util/assert.h"
#include "util/counter.h"
#include "util/sample.h"

namespace {
// TODO for the setting of sample rate 44100Hz, input/output buffer size
//  of 1024 frames (2048 samples) can be this value probably just 4096.
static constexpr SINT kReservedFrames = 8192;
} // anonymous namespace

PitchShiftEffect::PitchShiftEffect()
        : m_fullRingBuffer(false),
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

    const double pitchParameter = m_pPitchParameter->value();

    const double pitch = 1.0 + [=] {
        if (pitchParameter < 0.0) {
            return pitchParameter / 2.0;
        } else {
            return pitchParameter;
        }
    }();

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
