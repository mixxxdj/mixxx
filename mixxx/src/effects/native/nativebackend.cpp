
#include "effects/native/nativebackend.h"

#include "effects/native/flangereffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    m_effectManifests.append(FlangerEffect::getEffectManifest());
}

NativeBackend::~NativeBackend() {

}

const QList<EffectManifest> NativeBackend::getAvailableEffects() const {
    return m_effectManifests;
}

EffectPointer NativeBackend::instantiateEffect(const EffectManifest& manifest) {
    return EffectPointer();
}
