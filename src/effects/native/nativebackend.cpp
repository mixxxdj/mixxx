#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    FlangerEffect flanger;
    const EffectManifest& flanger_manifest = flanger.getManifest();
    m_effectManifests.append(flanger_manifest);
    registerEffect<FlangerProcessor>(flanger.getId(), flanger_manifest);

    BitCrusherEffect bitcrusher;
    const EffectManifest& bitcrusher_manifest = bitcrusher.getManifest();
    m_effectManifests.append(bitcrusher_manifest);
    registerEffect<BitCrusherProcessor>(bitcrusher.getId(),
                                        bitcrusher_manifest);
}

NativeBackend::~NativeBackend() {
    qDebug() << debugString() << "destroyed";
}
