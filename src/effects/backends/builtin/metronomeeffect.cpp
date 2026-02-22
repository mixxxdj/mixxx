#include "metronomeeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "metronomeclick.h"
#include "util/math.h"
#include "util/sample.h"

namespace {

void playMonoSamples(std::span<const CSAMPLE> monoSource, std::span<CSAMPLE> output) {
    const auto outputBufferFrames = output.size() / mixxx::kEngineChannelCount;
    SINT framesPlayed = std::min(monoSource.size(), outputBufferFrames);
    SampleUtil::addMonoToStereo(output.data(), monoSource.data(), framesPlayed);
}

double framesPerBeat(mixxx::audio::SampleRate sampleRate, double bpm) {
    double framesPerMinute = sampleRate * 60;
    return framesPerMinute / bpm;
}

} // namespace

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
    pManifest->setEffectRampsFromDry(true);

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

    auto output = std::span<CSAMPLE>(pOutput, engineParameters.samplesPerBuffer());

    MetronomeGroupState* gs = pGroupState;

    const std::span<const CSAMPLE> click = clickForSampleRate(engineParameters.sampleRate());
    SINT clickSize = click.size();

    if (pOutput != pInput) {
        SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
    }

    const bool shouldSync = m_pSyncParameter->toBool();

    if (enableState == EffectEnableState::Enabling) {
        if (shouldSync && groupFeatures.beat_fraction_buffer_end.has_value()) {
            // Skip first click and sync phase
            gs->m_framesSinceClickStart = clickSize;
        } else {
            // click right away after enabling
            gs->m_framesSinceClickStart = 0;
        }
    }

    if (gs->m_framesSinceClickStart < clickSize) {
        // In click region, write remaining click frames.
        playMonoSamples(click.subspan(gs->m_framesSinceClickStart), output);
    }

    double bufferEnd = gs->m_framesSinceClickStart + engineParameters.framesPerBuffer();

    double nextClickStart = bufferEnd; // default to "no new click";
    if (shouldSync && groupFeatures.beat_fraction_buffer_end.has_value()) {
        // Sync enabled and have a track with beats
        if (groupFeatures.beat_length.has_value() &&
                groupFeatures.beat_length->scratch_rate != 0.0) {
            double beatLength = groupFeatures.beat_length->seconds * engineParameters.sampleRate() /
                    groupFeatures.beat_length->scratch_rate;
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
            // no transport, continue until the current click has been fully played
            if (gs->m_framesSinceClickStart < clickSize) {
                gs->m_framesSinceClickStart += engineParameters.framesPerBuffer();
            }
            return;
        }
    } else {
        nextClickStart = framesPerBeat(engineParameters.sampleRate(), m_pBpmParameter->value());
    }

    if (bufferEnd > nextClickStart) {
        // We need to start a new click
        SINT outputOffset = static_cast<SINT>(nextClickStart) - gs->m_framesSinceClickStart;
        if (outputOffset > 0 && outputOffset < engineParameters.framesPerBuffer()) {
            playMonoSamples(click, output.subspan(outputOffset * 2));
        }
        // Due to seeking, we may have missed the start position of the click.
        // We pretend that it has been played to stay in phase
        gs->m_framesSinceClickStart = -outputOffset;
    }
    gs->m_framesSinceClickStart += engineParameters.framesPerBuffer();
}
