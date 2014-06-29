#include <QtDebug>

#include "effects/native/bitcrushereffect.h"
#include "effects/native/echoeffect.h"
#include "effects/native/filtereffect.h"
#include "effects/native/flangereffect.h"
#include "effects/native/lightweighteq.h"
#include "effects/native/nativebackend.h"
#include "effects/native/reverbeffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    registerEffect<FlangerEffect>();
    registerEffect<BitCrusherEffect>();
    registerEffect<FilterEffect>();
    registerEffect<ReverbEffect>();
    registerEffect<EchoEffect>();
    registerEffect<LightweightEQ>();
}

NativeBackend::~NativeBackend() {
    //qDebug() << debugString() << "destroyed";
}
