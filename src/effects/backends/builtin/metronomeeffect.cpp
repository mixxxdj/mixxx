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
    Q_UNUSED(pInput);

    MetronomeGroupState* gs = pGroupState;

    if (enableState == EffectEnableState::Disabled) {
        gs->m_framesSinceClickStart = 0;
        return;
    }

    SINT clickSize = kClickSize44100;
    const CSAMPLE* click = kClick44100;
    if (engineParameters.sampleRate() >= 96000) {
        clickSize = kClickSize96000;
        click = kClick96000;
    } else if (engineParameters.sampleRate() >= 48000) {
        clickSize = kClickSize48000;
        click = kClick48000;
    }

    SINT maxFrames;
    if (m_pSyncParameter->toBool() && groupFeatures.has_beat_length_sec) {
        maxFrames = static_cast<SINT>(
                engineParameters.sampleRate() * groupFeatures.beat_length_sec);
        if (groupFeatures.has_beat_fraction) {
            const auto currentFrame = static_cast<SINT>(
                    maxFrames * groupFeatures.beat_fraction);
            if (maxFrames > clickSize &&
                    currentFrame > clickSize &&
                    currentFrame < maxFrames - clickSize &&
                    gs->m_framesSinceClickStart > clickSize) {
                // plays a single click on low speed
                gs->m_framesSinceClickStart = currentFrame;
            }
        }
    } else {
        maxFrames = static_cast<SINT>(
                engineParameters.sampleRate() * 60 / m_pBpmParameter->value());
    }

    SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());

    if (gs->m_framesSinceClickStart < clickSize) {
        // still in click region, write remaining click frames.
        const SINT copyFrames =
                math_min(engineParameters.framesPerBuffer(),
                        clickSize - gs->m_framesSinceClickStart);
        SampleUtil::addMonoToStereo(pOutput, &click[gs->m_framesSinceClickStart], copyFrames);
    }

    gs->m_framesSinceClickStart += engineParameters.framesPerBuffer();

    if (gs->m_framesSinceClickStart > maxFrames) {
        // overflow, all overflowed frames are the start of a new click sound
        gs->m_framesSinceClickStart -= maxFrames;
        while (gs->m_framesSinceClickStart > engineParameters.framesPerBuffer()) {
            // loop into a valid region, this happens if the maxFrames was lowered
            gs->m_framesSinceClickStart -= engineParameters.framesPerBuffer();
        }

        // gs->m_framesSinceClickStart matches now already at the first frame
        // of next pOutput.
        const unsigned int outputOffset =
                engineParameters.framesPerBuffer() - gs->m_framesSinceClickStart;
        const unsigned int copyFrames =
                math_min(gs->m_framesSinceClickStart, clickSize);
        SampleUtil::addMonoToStereo(&pOutput[outputOffset * 2], click, copyFrames);
    }
}
