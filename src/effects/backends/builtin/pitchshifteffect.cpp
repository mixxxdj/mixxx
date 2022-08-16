/*
Rubber Band Library
An audio time-stretching and pitch-shifting library.
Copyright 2007-2022 Particular Programs Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.  See the file
COPYING included with this distribution for more information.

Alternatively, if you have a valid commercial licence for the
Rubber Band Library obtained by agreement with the copyright
holders, you may redistribute and/or modify it under the terms
described in that licence.

If you wish to distribute code using the Rubber Band Library
under terms other than those of the GNU General Public License,
you must obtain a valid commercial licence before doing so.
*/

/*
 * The PitchShiftEffect::processChannel function main algorithm
 * is based on the public ladspa-lv2 example implementation
 * by RubberBand.
 */

#include "effects/backends/builtin/pitchshifteffect.h"

#include "util/sample.h"

namespace {
static constexpr SINT kBlockFrames = 1024;
static constexpr SINT kPrefilledFrames = 8192;
} // anonymous namespace

PitchShiftEffect::PitchShiftEffect()
        : m_prevPitch(1.0),
          m_groupDelayFrames(kPrefilledFrames) {
}

PitchShiftGroupState::PitchShiftGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters) {
    initializeBuffer(engineParameters);
    audioParametersChanged(engineParameters);

    SampleUtil::clear(m_retrieveBuffer[0], kPrefilledFrames);
    SampleUtil::clear(m_retrieveBuffer[1], kPrefilledFrames);

    m_pRubberBand->process(m_retrieveBuffer, kPrefilledFrames, false);
}

PitchShiftGroupState::~PitchShiftGroupState() {
    SampleUtil::free(m_retrieveBuffer[0]);
    SampleUtil::free(m_retrieveBuffer[1]);
    SampleUtil::free(m_interleavedBuffer);
}

void PitchShiftGroupState::initializeBuffer(
        const mixxx::EngineParameters& engineParameters) {
    m_outputBuffer = std::make_unique<CircularBuffer<CSAMPLE>>(engineParameters.samplesPerBuffer());

    m_retrieveBuffer[0] = SampleUtil::alloc(
            math_max(engineParameters.framesPerBuffer(), kPrefilledFrames));
    m_retrieveBuffer[1] = SampleUtil::alloc(
            math_max(engineParameters.framesPerBuffer(), kPrefilledFrames));

    m_interleavedBuffer = SampleUtil::alloc(engineParameters.samplesPerBuffer());
}

void PitchShiftGroupState::audioParametersChanged(
        const mixxx::EngineParameters& engineParameters) {
    m_pRubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
            engineParameters.sampleRate(),
            engineParameters.channelCount(),
            RubberBand::RubberBandStretcher::OptionProcessRealTime |
                    RubberBand::RubberBandStretcher::OptionPitchHighConsistency);
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

    const SINT framesPerBuffer = engineParameters.framesPerBuffer();
    const mixxx::audio::ChannelCount channelCount = engineParameters.channelCount();

    const double pitchParameter = m_pPitchParameter->value();

    const double pitch = 1.0 + [=] {
        if (pitchParameter < 0.0) {
            return pitchParameter / 2.0;
        } else {
            return pitchParameter;
        }
    }();

    if (pitch != m_prevPitch) {
        pState->m_pRubberBand->setPitchScale(pitch);
        m_prevPitch = pitch;
    }

    SampleUtil::deinterleaveBuffer(
            pState->m_retrieveBuffer[0],
            pState->m_retrieveBuffer[1],
            pInput,
            framesPerBuffer);

    SINT offsetFrames = 0;

    while (offsetFrames < framesPerBuffer) {
        SINT blockFrames = kBlockFrames;

        if (offsetFrames + blockFrames > framesPerBuffer) {
            blockFrames = framesPerBuffer - offsetFrames;
        }

        SINT processedFrames = 0;

        while (processedFrames < blockFrames) {
            // RubberBand works with multichannel Sample FRAMES.
            SINT requiredFrames = pState->m_pRubberBand->getSamplesRequired();
            SINT toProcessFrames = math_min(blockFrames - processedFrames, requiredFrames);

            for (int c = 0; c < channelCount; ++c) {
                pState->m_inputSamples[c] =
                        pState->m_retrieveBuffer[c] +
                        offsetFrames + processedFrames;
            }

            pState->m_pRubberBand->process(
                    pState->m_inputSamples,
                    toProcessFrames,
                    false);

            processedFrames += toProcessFrames;

            SINT availableFrames = pState->m_pRubberBand->available();
            SINT writableFrames = pState->m_outputBuffer->getWriteSpace() / channelCount;

            if (availableFrames > writableFrames) {
                qDebug() << "PitchShiftEffect::processChannel: buffer is not large enough";
                availableFrames = writableFrames;
            }

            SINT retrievedFrames = pState->m_pRubberBand->retrieve(
                    pState->m_retrieveBuffer, availableFrames);
            SampleUtil::interleaveBuffer(pState->m_interleavedBuffer,
                    pState->m_retrieveBuffer[0],
                    pState->m_retrieveBuffer[1],
                    retrievedFrames);

            pState->m_outputBuffer->write(pState->m_interleavedBuffer,
                    retrievedFrames * channelCount);
        }

        SINT toReadFrames = pState->m_outputBuffer->getReadSpace() / channelCount;
        SINT missingFrames = 0;

        if (toReadFrames < blockFrames) {
            missingFrames = blockFrames - toReadFrames;
            SampleUtil::fill(pOutput + offsetFrames * channelCount,
                    0.0f,
                    missingFrames * channelCount);

            m_groupDelayFrames += missingFrames;
        }

        SINT outputFrames = math_min(toReadFrames, blockFrames);
        pState->m_outputBuffer->read(
                pOutput + (offsetFrames + missingFrames) * channelCount,
                outputFrames * channelCount);

        offsetFrames += blockFrames;
    }
}
