#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"
#include "effects/native/butterworth8eqeffect.h"
#include "effects/native/graphiceqeffect.h"
#include "effects/native/filtereffect.h"
#include "effects/native/moogladder4filtereffect.h"
#ifndef __MACAPPSTORE__
#include "effects/native/reverbeffect.h"
#endif
#include "effects/native/echoeffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    registerEffect<FlangerEffect>();
    registerEffect<BitCrusherEffect>();
    registerEffect<FilterEffect>();
    registerEffect<MoogLadder4FilterEffect>();
#ifndef __MACAPPSTORE__
    registerEffect<ReverbEffect>();
#endif
    registerEffect<EchoEffect>();
    registerEffect<Butterworth8EQEffect>();
    registerEffect<GraphicEQEffect>();
}

NativeBackend::~NativeBackend() {
    //qDebug() << debugString() << "destroyed";
}
