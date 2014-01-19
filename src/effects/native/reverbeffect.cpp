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
    manifest.setDescription("TODO");

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("time");
    time->setName(QObject::tr("time"));
    time->setDescription(QObject::tr(""));
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
    damping->setDescription(QObject::tr(""));
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
//    CSAMPLE time = m_pTimeParameter ?
//            m_pTimeParameter->value().toDouble() : 1.0f;
//    CSAMPLE damping = m_pDampingParameter ?
//            m_pDampingParameter->value().toDouble() : 0.5f;

    // do stuff.

}
