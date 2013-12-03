#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    registerEffect<FlangerEffect>();
    registerEffect<BitCrusherEffect>();
}

NativeBackend::~NativeBackend() {
    qDebug() << debugString() << "destroyed";
}
