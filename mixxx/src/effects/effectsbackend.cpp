
#include "effects/effectsbackend.h"

EffectsBackend::EffectsBackend(QObject* pParent, QString name)
        : QObject(pParent),
          m_name(name) {
}

EffectsBackend::~EffectsBackend() {
}

const QString EffectsBackend::getName() const {
    return m_name;
}
