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
#include "effects/builtin/autopaneffect.h"
#include "effects/builtin/echoeffect.h"
#include "effects/builtin/loudnesscontoureffect.h"
#include "effects/builtin/metronomeeffect.h"
#include "effects/builtin/phasereffect.h"
#include "effects/builtin/tremoloeffect.h"
#include "effects/builtin/whitenoiseeffect.h"

BuiltInBackend::BuiltInBackend() {
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
    registerEffect<WhiteNoiseEffect>();
    registerEffect<TremoloEffect>();
}

std::unique_ptr<EffectProcessor> BuiltInBackend::createProcessor(
        const EffectManifestPointer pManifest) const {
    VERIFY_OR_DEBUG_ASSERT(m_registeredEffects.contains(pManifest->id())) {
        return nullptr;
    }
    return m_registeredEffects[pManifest->id()].instantiator();
}

BuiltInBackend::~BuiltInBackend() {
    //qDebug() << debugString() << "destroyed";
    m_registeredEffects.clear();
    m_effectIds.clear();
}

void BuiltInBackend::registerEffectInner(
        const QString& id,
        EffectManifestPointer pManifest,
        EffectProcessorInstantiator instantiator) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredEffects.contains(id)) {
        return;
    }

    pManifest->setBackendType(getType());

    m_registeredEffects[id] = RegisteredEffect{pManifest, instantiator};
    m_effectIds.append(id);
}

const QList<QString> BuiltInBackend::getEffectIds() const {
    return m_effectIds;
}

EffectManifestPointer BuiltInBackend::getManifest(const QString& effectId) const {
    VERIFY_OR_DEBUG_ASSERT(m_registeredEffects.contains(effectId)) {
        return EffectManifestPointer();
    }
    return m_registeredEffects.value(effectId).pManifest;
}

const QList<EffectManifestPointer> BuiltInBackend::getManifests() const {
    QList<EffectManifestPointer> list;
    for (const auto& registeredEffect : m_registeredEffects) {
        list.append(registeredEffect.pManifest);
    }
    return list;
}

bool BuiltInBackend::canInstantiateEffect(const QString& effectId) const {
    return m_registeredEffects.contains(effectId);
}
