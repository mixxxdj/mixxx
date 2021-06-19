#include "metronomeeffect.h"

#include <QtDebug>

#include "metronomeclick.h"
#include "util/experiment.h"
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
    period->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    period->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    period->setMinimum(40);
    period->setDefault(120);
    period->setMaximum(208);


    // Period unit
    EffectManifestParameterPointer periodUnit = pManifest->addParameter();
    periodUnit->setId("sync");
    periodUnit->setName(QObject::tr("Sync"));
    periodUnit->setDescription(QObject::tr("Synchronizes the BPM with the track if it can be retrieved"));
    periodUnit->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    periodUnit->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    periodUnit->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    periodUnit->setDefault(1);
    periodUnit->setMinimum(0);
    periodUnit->setMaximum(1);

    return pManifest;
}

MetronomeEffect::MetronomeEffect(EngineEffect* pEffect)
    : m_pBpmParameter(pEffect->getParameterById("bpm")),
      m_pSyncParameter(pEffect->getParameterById("sync")) {
}

MetronomeEffect::~MetronomeEffect() {
}

void MetronomeEffect::processChannel(
        const ChannelHandle& handle,
        MetronomeGroupState* pGroupState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(pInput);

    MetronomeGroupState* gs = pGroupState;

    if (enableState == EffectEnableState::Disabled) {
        gs->m_framesSinceClickStart = 0;
        return;
    }

    SINT clickSize = kClickSize44100;
    const CSAMPLE* click = kClick44100;
    if (bufferParameters.sampleRate() >= 96000) {
        clickSize = kClickSize96000;
        click = kClick96000;
    } else if (bufferParameters.sampleRate() >= 48000) {
        clickSize = kClickSize48000;
        click = kClick48000;
    }

    SINT maxFrames;
    if (m_pSyncParameter->toBool() && groupFeatures.has_beat_length_sec) {
        maxFrames = static_cast<SINT>(
                bufferParameters.sampleRate() * groupFeatures.beat_length_sec);
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
                bufferParameters.sampleRate() * 60 / m_pBpmParameter->value());
    }

    SampleUtil::copy(pOutput, pInput, bufferParameters.samplesPerBuffer());

    if (gs->m_framesSinceClickStart < clickSize) {
        // still in click region, write remaining click frames.
        const SINT copyFrames =
                math_min(bufferParameters.framesPerBuffer(),
                        clickSize - gs->m_framesSinceClickStart);
        SampleUtil::addMonoToStereo(pOutput, &click[gs->m_framesSinceClickStart],
                copyFrames);
    }

    gs->m_framesSinceClickStart += bufferParameters.framesPerBuffer();

    if (gs->m_framesSinceClickStart > maxFrames) {
        // overflow, all overflowed frames are the start of a new click sound
        gs->m_framesSinceClickStart -= maxFrames;
        while (gs->m_framesSinceClickStart > bufferParameters.framesPerBuffer()) {
            // loop into a valid region, this happens if the maxFrames was lowered
            gs->m_framesSinceClickStart -= bufferParameters.framesPerBuffer();
        }

        // gs->m_framesSinceClickStart matches now already at the first frame
        // of next pOutput.
        const unsigned int outputOffset =
                bufferParameters.framesPerBuffer() - gs->m_framesSinceClickStart;
        const unsigned int copyFrames =
                math_min(gs->m_framesSinceClickStart, clickSize);
        SampleUtil::addMonoToStereo(&pOutput[outputOffset * 2], click,
                copyFrames);
    }
}
