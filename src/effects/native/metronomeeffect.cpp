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
EffectManifest MetronomeEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Metronome"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Adds a metronome click sound to the stream"));

    // Period
    // The maximum is at 128 + 1 allowing 128 as max value and
    // enabling us to pause time when the parameter is above
    EffectManifestParameter* period = manifest.addParameter();
    period->setId("bpm");
    period->setName(QObject::tr("BPM"));
    period->setDescription(QObject::tr("Set the beats per minute value of the click sound"));
    period->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    period->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    period->setMinimum(40);
    period->setMaximum(129.0);
    period->setDefault(208);


    // Period unit
    EffectManifestParameter* periodUnit = manifest.addParameter();
    periodUnit->setId("sync");
    periodUnit->setName(QObject::tr("Sync"));
    periodUnit->setDescription(QObject::tr("Synchronizes the BPM with the track if it can be retrieved"));
    periodUnit->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    periodUnit->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    periodUnit->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    periodUnit->setDefault(1);
    periodUnit->setMinimum(0);
    periodUnit->setMaximum(1);

    return manifest;
}

MetronomeEffect::MetronomeEffect(EngineEffect* pEffect, const EffectManifest& manifest) {
    Q_UNUSED(manifest);
}

MetronomeEffect::~MetronomeEffect() {
}

void MetronomeEffect::processChannel(const ChannelHandle& handle, MetronomeGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);

    if (enableState == EffectProcessor::DISABLED) {
        return;
    }

    MetronomeGroupState* gs = pGroupState;

    SampleUtil::clear(pOutput, numSamples);

    /*
    if (gs->m_framesSinceClickStart < sampleRate) {
        gs->m_framesSinceClickStart -= sampleRate;
    }
*/
    const unsigned int numFrames = numSamples / 2;

    if (gs->m_framesSinceClickStart < kClickSize) {
        // Write remaining click frames.
        const unsigned int copyFrames =
                math_min(numFrames, kClickSize - gs->m_framesSinceClickStart);
        SampleUtil::copyMonoToDualMono(pOutput, &kClick[gs->m_framesSinceClickStart],
                copyFrames);
    }

    gs->m_framesSinceClickStart += numFrames;

    if (gs->m_framesSinceClickStart > sampleRate) {
        // overflow, start new click sound
        gs->m_framesSinceClickStart -= sampleRate;
        const unsigned int copyFrames =
                math_min(gs->m_framesSinceClickStart, kClickSize);
        const unsigned int outputOffset =
                numFrames - gs->m_framesSinceClickStart;
        SampleUtil::copyMonoToDualMono(&pOutput[outputOffset * 2], kClick,
                copyFrames);
    }
}

