#include "metronomeeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "metronomeclick.h"
#include "util/math.h"
#include "util/sample.h"

// static
QString MetronomeEffect::getId() {
    return "org.mixxx.effects.metronome";
}

// static
EffectManifestPointer MetronomeEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Metronome"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr("Adds a metronome click sound to the stream"));

    // Period
    // The maximum is at 128 + 1 allowing 128 as max value and
    // enabling us to pause time when the parameter is above
    EffectManifestParameterPointer period = pManifest->addParameter();
    period->setId("bpm");
    period->setName(QObject::tr("BPM"));
    period->setDescription(QObject::tr("Set the beats per minute value of the click sound"));
    period->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    period->setRange(40, 120, 208);

    // Period unit
    EffectManifestParameterPointer periodUnit = pManifest->addParameter();
    periodUnit->setId("sync");
    periodUnit->setName(QObject::tr("Sync"));
    periodUnit->setDescription(QObject::tr(
            "Synchronizes the BPM with the track if it can be retrieved"));
    periodUnit->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    periodUnit->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    periodUnit->setRange(0, 1, 1);

    return pManifest;
}

void MetronomeEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pBpmParameter = parameters.value("bpm");
    m_pSyncParameter = parameters.value("sync");
}

void MetronomeEffect::processChannel(
        MetronomeGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    if (enableState == EffectEnableState::Disabled) {
        // assume click is fully played
        return;
    }

    MetronomeGroupState* gs = pGroupState;

    SINT clickSize = kClickSize44100;
    const CSAMPLE* click = kClick44100;
    if (engineParameters.sampleRate() >= 96000) {
        clickSize = kClickSize96000;
        click = kClick96000;
    } else if (engineParameters.sampleRate() >= 48000) {
        clickSize = kClickSize48000;
        click = kClick48000;
    }

    if (pOutput != pInput) {
        SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
    }

    if (enableState == EffectEnableState::Enabling) {
        if (m_pSyncParameter->toBool() && groupFeatures.beat_fraction_buffer_end.has_value()) {
            // Skip first click and sync phase
            gs->m_framesSinceClickStart = clickSize;
        } else {
            // click right away after enabling
            gs->m_framesSinceClickStart = 0;
        }
    }

    if (gs->m_framesSinceClickStart < clickSize) {
        // In click region, write remaining click frames.
        const SINT copyFrames =
                math_min(engineParameters.framesPerBuffer(),
                        clickSize - gs->m_framesSinceClickStart);
        SampleUtil::addMonoToStereo(pOutput, &click[gs->m_framesSinceClickStart], copyFrames);
    }

    double bufferEnd = gs->m_framesSinceClickStart + engineParameters.framesPerBuffer();

    double nextClickStart = bufferEnd; // default to "no new click";
    if (m_pSyncParameter->toBool() && groupFeatures.beat_length_frames.has_value()) {
        if (groupFeatures.beat_fraction_buffer_end.has_value()) {
            double beatLength = *groupFeatures.beat_length_frames;
            if (groupFeatures.scratch_rate.has_value()) {
                if (*groupFeatures.scratch_rate != 0.0) {
                    beatLength /= *groupFeatures.scratch_rate;
                } else {
                    beatLength = 0;
                }
            }

            double beatToBufferEnd;
            if (beatLength > 0) {
                beatToBufferEnd =
                        beatLength *
                        *groupFeatures.beat_fraction_buffer_end;
            } else {
                beatToBufferEnd =
                        beatLength * -1 *
                        (1 - *groupFeatures.beat_fraction_buffer_end);
            }

            if (bufferEnd > beatToBufferEnd) {
                // We have a new beat before the current buffer ends
                nextClickStart = bufferEnd - beatToBufferEnd;
            }
        } else {
            nextClickStart = *groupFeatures.beat_length_frames;
        }
    } else {
        nextClickStart = engineParameters.sampleRate() * 60 / m_pBpmParameter->value();
    }

    if (bufferEnd > nextClickStart) {
        // We need to start a new click
        SINT outputOffset = static_cast<SINT>(nextClickStart) - gs->m_framesSinceClickStart;
        if (outputOffset > 0 && outputOffset < engineParameters.framesPerBuffer()) {
            const SINT copyFrames =
                    math_min(engineParameters.framesPerBuffer() - outputOffset, clickSize);
            SampleUtil::addMonoToStereo(&pOutput[outputOffset * 2], &click[0], copyFrames);
            gs->m_framesSinceClickStart = -outputOffset;
        }
    }
    if (gs->m_framesSinceClickStart < engineParameters.framesPerBuffer() + clickSize) {
        gs->m_framesSinceClickStart += engineParameters.framesPerBuffer();
    }
}
