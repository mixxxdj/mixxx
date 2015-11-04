#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"
#include "effects/native/linkwitzriley8eqeffect.h"
#include "effects/native/bessel8lvmixeqeffect.h"
#include "effects/native/bessel4lvmixeqeffect.h"
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
    registerEffect<LinkwitzRiley8EQEffect>();
    registerEffect<Bessel4LVMixEQEffect>();
    registerEffect<Bessel8LVMixEQEffect>();
    registerEffect<GraphicEQEffect>();
}

NativeBackend::~NativeBackend() {
    //qDebug() << debugString() << "destroyed";
}
