#include "effects/builtin/builtinbackend.h"

#include <QtDebug>

#include "effects/builtin/balanceeffect.h"
#include "effects/builtin/bessel4lvmixeqeffect.h"
#include "effects/builtin/bessel8lvmixeqeffect.h"
#include "effects/builtin/biquadfullkilleqeffect.h"
#include "effects/builtin/bitcrushereffect.h"
#include "effects/builtin/filtereffect.h"
#include "effects/builtin/flangereffect.h"
#include "effects/builtin/graphiceqeffect.h"
#include "effects/builtin/linkwitzriley8eqeffect.h"
#include "effects/builtin/moogladder4filtereffect.h"
#include "effects/builtin/parametriceqeffect.h"
#include "effects/builtin/threebandbiquadeqeffect.h"
#include "moc_builtinbackend.cpp"
#ifndef __MACAPPSTORE__
#include "effects/builtin/reverbeffect.h"
#endif
#include "effects/builtin/echoeffect.h"
#include "effects/builtin/autopaneffect.h"
#include "effects/builtin/phasereffect.h"
#include "effects/builtin/loudnesscontoureffect.h"
#include "effects/builtin/metronomeeffect.h"
#include "effects/builtin/tremoloeffect.h"

BuiltInBackend::BuiltInBackend(QObject* pParent)
        : EffectsBackend(pParent, EffectBackendType::BuiltIn) {
    // Keep this list in a reasonable order
    // Mixing EQs
    registerEffect<Bessel4LVMixEQEffect>();
    registerEffect<Bessel8LVMixEQEffect>();
    registerEffect<LinkwitzRiley8EQEffect>();
    registerEffect<ThreeBandBiquadEQEffect>();
    registerEffect<BiquadFullKillEQEffect>();
    // Compensations EQs
    registerEffect<GraphicEQEffect>();
    registerEffect<ParametricEQEffect>();
    registerEffect<LoudnessContourEffect>();
    // Fading Effects
    registerEffect<FilterEffect>();
    registerEffect<MoogLadder4FilterEffect>();
    registerEffect<BitCrusherEffect>();
    registerEffect<BalanceEffect>();
    // Fancy effects
    registerEffect<FlangerEffect>();
    registerEffect<EchoEffect>();
    registerEffect<AutoPanEffect>();
#ifndef __MACAPPSTORE__
    registerEffect<ReverbEffect>();
#endif
    registerEffect<PhaserEffect>();
    registerEffect<MetronomeEffect>();
    registerEffect<TremoloEffect>();
}

BuiltInBackend::~BuiltInBackend() {
    //qDebug() << debugString() << "destroyed";
}
