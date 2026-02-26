
#include "effects/backends/builtin/builtinbackend.h"

#include "effects/backends/builtin/balanceeffect.h"
#include "effects/backends/builtin/bessel4lvmixeqeffect.h"
#include "effects/backends/builtin/bessel8lvmixeqeffect.h"
#include "effects/backends/builtin/biquadfullkilleqeffect.h"
#include "effects/backends/builtin/bitcrushereffect.h"
#include "effects/backends/builtin/filtereffect.h"
#include "effects/backends/builtin/flangereffect.h"
#include "effects/backends/builtin/graphiceqeffect.h"
#include "effects/backends/builtin/linkwitzriley8eqeffect.h"
#include "effects/backends/builtin/moogladder4filtereffect.h"
#include "effects/backends/builtin/parametriceqeffect.h"
#include "effects/backends/builtin/threebandbiquadeqeffect.h"
#ifndef __MACAPPSTORE__
#include "effects/backends/builtin/reverbeffect.h"
#endif
#include "effects/backends/builtin/autogaincontroleffect.h"
#include "effects/backends/builtin/autopaneffect.h"
#include "effects/backends/builtin/compressoreffect.h"
#include "effects/backends/builtin/distortioneffect.h"
#include "effects/backends/builtin/echoeffect.h"
#include "effects/backends/builtin/glitcheffect.h"
#include "effects/backends/builtin/loudnesscontoureffect.h"
#include "effects/backends/builtin/metronomeeffect.h"
#include "effects/backends/builtin/phasereffect.h"
#ifdef __RUBBERBAND__
#include "effects/backends/builtin/pitchshifteffect.h"
#endif
#include "effects/backends/builtin/tremoloeffect.h"
#include "effects/backends/builtin/whitenoiseeffect.h"

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
    registerEffect<WhiteNoiseEffect>();
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
#ifdef __RUBBERBAND__
    registerEffect<PitchShiftEffect>();
#endif
    registerEffect<DistortionEffect>();
    registerEffect<GlitchEffect>();
    registerEffect<CompressorEffect>();
    registerEffect<AutoGainControlEffect>();
}

std::unique_ptr<EffectProcessor> BuiltInBackend::createProcessor(
        const EffectManifestPointer pManifest) const {
    VERIFY_OR_DEBUG_ASSERT(m_registeredEffects.contains(pManifest->id())) {
        return nullptr;
    }
    return m_registeredEffects[pManifest->id()].instantiator();
}

BuiltInBackend::~BuiltInBackend() {
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
