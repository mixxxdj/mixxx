#include <QtDebug>
#include <QMutexLocker>

#include "effects/effect.h"
#include "effects/effectsbackend.h"

Effect::Effect(EffectsBackend* pBackend, const EffectManifest& manifest)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_pEffectsBackend(pBackend),
          m_manifest(manifest) {
    foreach (EffectManifestParameter parameter, m_manifest.parameters()) {
        EffectParameterPointer pParameter(new EffectParameter(this, parameter));
        m_parameters.append(pParameter);
        if (m_parametersById.contains(parameter.id())) {
            qDebug() << debugString() << "WARNING: Loaded EffectManifest that had parameters with duplicate IDs. Dropping one of them.";
        }
        m_parametersById[parameter.id()] = pParameter;
    }
}

Effect::~Effect() {
    qDebug() << debugString() << "destroyed";
    m_parameters.clear();
    m_parametersById.clear();
}

const EffectManifest& Effect::getManifest() const {
    QMutexLocker locker(&m_mutex);
    return m_manifest;
}

unsigned int Effect::numParameters() const {
    QMutexLocker locker(&m_mutex);
    return m_parameters.size();
}

EffectParameterPointer Effect::getParameterFromId(const QString id) const {
    QMutexLocker locker(&m_mutex);
    if (m_parametersById.contains(id)) {
        return m_parametersById[id];
    }
    qDebug() << debugString() << "parameterFromId" << "WARNING: parameter for id does not exist:" << id;
    return EffectParameterPointer();
}

EffectParameterPointer Effect::getParameter(unsigned int parameterNumber) {
    QMutexLocker locker(&m_mutex);
    if (parameterNumber >= m_parameters.size()) {
        qDebug() << debugString() << "WARNING: Invalid parameter index.";
        return EffectParameterPointer();
    }
    return m_parameters[parameterNumber];
}
