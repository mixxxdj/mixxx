
#include "effects/native/nativebackend.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {

}

NativeBackend::~NativeBackend() {

}

const QList<EffectManifest> NativeBackend::getAvailableEffects() const {
    return m_effectManifests;
}

EffectPointer NativeBackend::instantiateEffect(const EffectManifest& manifest) {
    return EffectPointer();
}
