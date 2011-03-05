#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    m_effectManifests.append(FlangerEffect::getEffectManifest());
}

NativeBackend::~NativeBackend() {
    qDebug() << debugString() << "destroyed";
    m_instantiatedEffects.clear();
}

const QList<EffectManifest> NativeBackend::getAvailableEffects() const {
    return m_effectManifests;
}

EffectPointer NativeBackend::instantiateEffect(const EffectManifest& manifest) {
    if (manifest.id() == "org.mixxx.effects.flanger") {
        EffectPointer flanger = EffectPointer(new FlangerEffect(this, manifest),
                                              &QObject::deleteLater);
        m_instantiatedEffects.append(flanger);
        return flanger;
    }
    return EffectPointer();
}
