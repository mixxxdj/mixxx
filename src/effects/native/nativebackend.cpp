#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    registerEffect<FlangerProcessor>();
    registerEffect<BitCrusherProcessor>();
}

NativeBackend::~NativeBackend() {
    qDebug() << debugString() << "destroyed";
}
