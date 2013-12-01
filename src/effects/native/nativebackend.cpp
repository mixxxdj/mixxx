#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    FlangerEffect flanger;
    const EffectManifest& flanger_manifest = flanger.getManifest();
    m_effectManifests.append(flanger_manifest);
    registerEffect<FlangerEffectProcessor>(flanger.getId(), flanger_manifest);
}

NativeBackend::~NativeBackend() {
    qDebug() << debugString() << "destroyed";
}
