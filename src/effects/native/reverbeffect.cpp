#include <QtDebug>

#include "effects/native/reverbeffect.h"
#include "effects/native/waveguide_nl.h"

#include "mathstuff.h"
#include "sampleutil.h"

#define RUN_WG(n, junct_a, junct_b) waveguide_nl_process_lin(pState->waveguide[n], junct_a - pState->out[n*2+1], junct_b - pState->out[n*2], pState->out+n*2, pState->out+n*2+1)

// static
QString ReverbEffect::getId() {
    return "org.mixxx.effects.reverb";
}

// static
EffectManifest ReverbEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Reverb"));
    manifest.setAuthor("The Mixxx Team, SWH Plugins");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("time");
    time->setName(QObject::tr("time"));
    time->setDescription(QObject::tr("Controls the RT60 time of the reverb. "
                         "Actually controls the size of the plate. The mapping "
                         "between plate size and RT60 time is just a "
                         "heuristic, so it's not very accurate."));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.1);
    time->setDefault(1.0);
    time->setMaximum(8.5);

    EffectManifestParameter* damping = manifest.addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("damping"));
    damping->setDescription(QObject::tr("Controls the degree that the surface "
                            "of the plate is damped."));
    damping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    damping->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    damping->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    damping->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    damping->setMinimum(0.0);
    damping->setDefault(0.5);
    damping->setMaximum(1.0);

    return manifest;
}

ReverbEffect::ReverbEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pTimeParameter(pEffect->getParameterById("time")),
          m_pDampingParameter(pEffect->getParameterById("damping")) {
}

ReverbEffect::~ReverbEffect() {
    qDebug() << debugString() << "destroyed";
}

void ReverbEffect::processGroup(const QString& group,
                                ReverbGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples) {
    CSAMPLE time = m_pTimeParameter ?
            m_pTimeParameter->value().toDouble() : 1.0f;
    CSAMPLE damping = m_pDampingParameter ?
            m_pDampingParameter->value().toDouble() : 0.5f;

    // TODO(owilliams) check ranges?

    unsigned long pos;
    const float scale = powf(time * 0.117647f, 1.34f);
    const float lpscale = 1.0f - damping * 0.93;

    for (pos=0; pos<8; pos++) {
        waveguide_nl_set_delay(pState->waveguide[pos],
                               pState->waveguide[pos]->size * scale);
    }
    for (pos=0; pos<4; pos++) {
        waveguide_nl_set_fc(pState->waveguide[pos], LP_INNER * lpscale);
    }
    for (; pos<8; pos++) {
        waveguide_nl_set_fc(pState->waveguide[pos], LP_OUTER * lpscale);
    }

    for (pos = 0; pos < numSamples - 1; pos+=2) {
        const float alpha =
                (pState->out[0] + pState->out[2] +
                 pState->out[4] + pState->out[6]) * 0.5f + pInput[pos];
        const float beta =
                (pState->out[1] + pState->out[9] + pState->out[14])
                * 0.666666666f;
        const float gamma =
                (pState->out[3] + pState->out[8] + pState->out[11])
                * 0.666666666f;
        const float delta =
                (pState->out[5] + pState->out[10] + pState->out[13])
                * 0.666666666f;
        const float epsilon =
                (pState->out[7] + pState->out[12] + pState->out[15])
                * 0.666666666f;

        RUN_WG(0, beta, alpha);
        RUN_WG(1, gamma, alpha);
        RUN_WG(2, delta, alpha);
        RUN_WG(3, epsilon, alpha);
        RUN_WG(4, beta, gamma);
        RUN_WG(5, gamma, delta);
        RUN_WG(6, delta, epsilon);
        RUN_WG(7, epsilon, beta);

        pOutput[pos] = beta + pInput[pos];
        pOutput[pos + 1] = beta + pInput[pos + 1];
    }
}
