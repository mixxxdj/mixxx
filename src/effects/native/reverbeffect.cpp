#include <QtDebug>

#include "effects/native/reverbeffect.h"
#include "effects/native/waveguide_nl.h"

#include "mathstuff.h"
#include "sampleutil.h"

const unsigned int kOutBufSize = 32;

// static
QString ReverbEffect::getId() {
    return "org.mixxx.effects.Reverb";
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
    time->setDescription("Controls the RT60 time of the reverb. Actually "
                         "controls the size of the plate. The mapping between "
                         "plate size and RT60 time is just a heuristic, so "
                         "it's not very accurate.");
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setDefault(0.1);
    time->setMinimum(1.0);
    time->setMaximum(8.5);

    EffectManifestParameter* damping = manifest.addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("damping"));
    damping->setDescription("Controls the degree that the surface of the plate "
                            "is damped.");
    damping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    damping->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    damping->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    damping->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    damping->setDefault(0.5);
    damping->setMinimum(0.0);
    damping->setMaximum(1.0);

    return manifest;
}

ReverbEffect::ReverbEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pTimeParameter(pEffect->getParameterById("time")),
          m_pDampingParameter(pEffect->getParameterById("damping")) {

    m_pWaveguide = (waveguide_nl**)malloc(8 * sizeof(waveguide_nl *));
    m_pWaveguide[0] = waveguide_nl_new(2389, LP_INNER, 0.04f, 0.0f);
    m_pWaveguide[1] = waveguide_nl_new(4742, LP_INNER, 0.17f, 0.0f);
    m_pWaveguide[2] = waveguide_nl_new(4623, LP_INNER, 0.52f, 0.0f);
    m_pWaveguide[3] = waveguide_nl_new(2142, LP_INNER, 0.48f, 0.0f);
    m_pWaveguide[4] = waveguide_nl_new(5597, LP_OUTER, 0.32f, 0.0f);
    m_pWaveguide[5] = waveguide_nl_new(3692, LP_OUTER, 0.89f, 0.0f);
    m_pWaveguide[6] = waveguide_nl_new(5611, LP_OUTER, 0.28f, 0.0f);
    m_pWaveguide[7] = waveguide_nl_new(3703, LP_OUTER, 0.29f, 0.0f);

    m_pOut = SampleUtil::alloc(kOutBufSize);
}

ReverbEffect::~ReverbEffect() {
    for (int i = 0; i < 8; i++) {
        waveguide_nl_reset(m_pWaveguide[i]);
    }
    SampleUtil::free(m_pOut);
    qDebug() << debugString() << "destroyed";
}

void ReverbEffect::process(const QString& group,
                            const CSAMPLE* pInput, CSAMPLE* pOutput,
                            const unsigned int numSamples) {
    GroupState& group_state = m_groupState[group];

    CSAMPLE time = m_pTimeParameter ?
            m_pTimeParameter->value().toDouble() : 1.0f;
    CSAMPLE damping = m_pDampingParameter ?
            m_pDampingParameter->value().toDouble() : 0.5f;

    // TODO(owilliams) check ranges?

    unsigned long pos;
    const float scale = powf(time * 0.117647f, 1.34f);
    const float lpscale = 1.0f - damping * 0.93;

    for (pos=0; pos<8; pos++) {
        waveguide_nl_set_delay(m_pWaveguide[pos],
                               m_pWaveguide[pos]->size * scale);
    }
    for (pos=0; pos<4; pos++) {
        waveguide_nl_set_fc(m_pWaveguide[pos], LP_INNER * lpscale);
    }
    for (; pos<8; pos++) {
        waveguide_nl_set_fc(m_pWaveguide[pos], LP_OUTER * lpscale);
    }

    for (pos = 0; pos < numSamples - 1; pos+=2) {
        const float alpha =
                (m_pOut[0] + m_pOut[2] + m_pOut[4] + m_pOut[6]) * 0.5f
                + pInput[pos];
        const float beta =
                (m_pOut[1] + m_pOut[9] + m_pOut[14]) * 0.666666666f;
        const float gamma =
                (m_pOut[3] + m_pOut[8] + m_pOut[11]) * 0.666666666f;
        const float delta =
                (m_pOut[5] + m_pOut[10] + m_pOut[13]) * 0.666666666f;
        const float epsilon =
                (m_pOut[7] + m_pOut[12] + m_pOut[15]) * 0.666666666f;

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
