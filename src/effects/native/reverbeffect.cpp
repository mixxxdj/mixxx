#include <QtDebug>

#include "effects/native/reverbeffect.h"

#include "sampleutil.h"

// static
QString ReverbEffect::getId() {
    return "org.mixxx.effects.reverb";
}

// static
EffectManifest ReverbEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Reverb"));
    manifest.setAuthor("The Mixxx Team, CAPS Plugins");
    manifest.setVersion("1.0");
    manifest.setDescription("This is a port of the GPL'ed CAPS Reverb plugin, "
            "which has the following description:"
            "This is based on some of the famous Stanford CCRMA reverbs "
            "(NRev, KipRev) all based on the Chowning/Moorer/Schroeder "
            "reverberators, which use networks of simple allpass and comb"
            "delay filters.");

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("bandwidth");
    time->setName(QObject::tr("Bandwidth"));
    time->setDescription(QObject::tr("Higher bandwidth values cause more "
            "bright (high-frequency) tones to be included"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.0005);
    time->setDefault(0.5);
    time->setMaximum(1.0);

    EffectManifestParameter* damping = manifest.addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("Damping"));
    damping->setDescription(QObject::tr("Higher damping values cause "
            "reverberations to die out more quickly."));
    damping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    damping->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    damping->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    damping->setMinimum(0.005);
    damping->setDefault(0.5);
    damping->setMaximum(1.0);

    return manifest;
}

ReverbEffect::ReverbEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pBandWidthParameter(pEffect->getParameterById("bandwidth")),
          m_pDampingParameter(pEffect->getParameterById("damping")) {
    Q_UNUSED(manifest);
}

ReverbEffect::~ReverbEffect() {
    //qDebug() << debugString() << "destroyed";
}

void ReverbEffect::processGroup(const QString& group,
                                ReverbGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    CSAMPLE bandwidth = m_pBandWidthParameter->value();
    CSAMPLE damping = m_pDampingParameter->value();

    // Flip value around.  Assumes max allowable is 1.0.
    damping = 1.0 - damping;

    bool params_changed = (damping != pState->prev_damping ||
                           bandwidth != pState->prev_bandwidth);

    pState->reverb.setBandwidth(bandwidth);
    pState->reverb.setDecay(damping);

    for (uint i = 0; i + 1 < numSamples; i += 2) {
        CSAMPLE mono_sample = (pInput[i] + pInput[i + 1]) / 2;
        CSAMPLE xl, xr;

        // sample_t is typedefed to be the same as CSAMPLE, so no cast needed.
        pState->reverb.process(mono_sample, damping, &xl, &xr);

        pOutput[i] = xl;
        pOutput[i + 1] = xr;
    }

    if (params_changed) {
        pState->reverb.setBandwidth(pState->prev_bandwidth);
        pState->reverb.setDecay(pState->prev_damping);

        for (uint i = 0; i + 1 < numSamples; i += 2) {
            CSAMPLE mono_sample = (pInput[i] + pInput[i + 1]) / 2;
            CSAMPLE xl, xr;

            pState->reverb.process(mono_sample, pState->prev_damping, &xl, &xr);

            pState->crossfade_buffer[i] = xl;
            pState->crossfade_buffer[i + 1] = xr;
        }

        pState->prev_bandwidth = bandwidth;
        pState->prev_damping = damping;

        SampleUtil::linearCrossfadeBuffers(
                pOutput, pOutput, pState->crossfade_buffer, numSamples);
    }
}
