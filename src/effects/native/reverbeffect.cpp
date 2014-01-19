#include <QtDebug>

#include "effects/native/reverbeffect.h"

#include "mathstuff.h"
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
    time->setName(QObject::tr("bandwidth"));
    time->setDescription(QObject::tr("Higher bandwidth values cause more "
            "bright (high-frequency) tones to be included"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.0005);
    time->setDefault(0.5);
    time->setMaximum(1.0);

    EffectManifestParameter* damping = manifest.addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("damping"));
    damping->setDescription(QObject::tr("Higher damping values cause "
            "reverberations to die out more quickly."));
    damping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    damping->setValueHint(EffectManifestParameter::VALUE_FLOAT);
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
}

ReverbEffect::~ReverbEffect() {
    qDebug() << debugString() << "destroyed";
}

void ReverbEffect::processGroup(const QString& group,
                                ReverbGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples) {
    CSAMPLE bandwidth = m_pBandWidthParameter ?
            m_pBandWidthParameter->value().toDouble() : 1.0f;
    CSAMPLE damping = m_pDampingParameter ?
            m_pDampingParameter->value().toDouble() : 0.5f;

    // Flip value around.  Assumes max allowable is 1.0.
    damping = 1.0 - damping;

    double damp_inc = 0.0;

    if (damping != pState->prev_damping) {
        damp_inc = (damping - pState->prev_damping) / (numSamples / 2);
    }

    pState->reverb.setBandwidth(bandwidth);
    pState->reverb.setDecay(damping);

    for (uint i = 0; i + 1 < numSamples; i += 2) {
        CSAMPLE mono_sample = (pInput[i] + pInput[i + 1]) / 2;
        CSAMPLE xl, xr;

        // sample_t is typedefed to be the same as CSAMPLE, so no cast needed.
        pState->reverb.process(mono_sample, damping, &xl, &xr);

        pOutput[i] = xl;
        pOutput[i + 1] = xr;

        if (damp_inc != 0.0) {
            pState->prev_damping += damp_inc;
            pState->reverb.setDecay(pState->prev_damping);
        }
    }
}
