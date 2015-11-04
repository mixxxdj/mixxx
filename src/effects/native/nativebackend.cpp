#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"
#include "effects/native/butterwortheqeffect.h"
#include "effects/native/filtereffect.h"
#ifndef __MACAPPSTORE__
#include "effects/native/reverbeffect.h"
#endif
#include "effects/native/echoeffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    registerEffect<FlangerEffect>();
    registerEffect<BitCrusherEffect>();
    registerEffect<FilterEffect>();
#ifndef __MACAPPSTORE__
    registerEffect<ReverbEffect>();
#endif
    registerEffect<EchoEffect>();
    registerEffect<ButterworthEQEffect>();
}

NativeBackend::~NativeBackend() {
    //qDebug() << debugString() << "destroyed";
}
