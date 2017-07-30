#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include "control/controlpotmeter.h"
#include "effects/native/autopaneffect.h"
#include "effects/native/bessel4lvmixeqeffect.h"
#include "effects/native/bessel8lvmixeqeffect.h"
#include "effects/native/bitcrushereffect.h"
#include "effects/native/echoeffect.h"
#include "effects/native/filtereffect.h"
#include "effects/native/flangereffect.h"
#include "effects/native/graphiceqeffect.h"
#include "effects/native/linkwitzriley8eqeffect.h"
#include "effects/native/moogladder4filtereffect.h"
#include "effects/native/phasereffect.h"
#include "effects/native/reverbeffect.h"
#include "engine/channelhandle.h"
#include "engine/effects/groupfeaturestate.h"
#include "test/mixxxtest.h"
#include "util/samplebuffer.h"

namespace {

template <class EffectType>
void benchmarkNativeEffectDefaultParameters(const unsigned int sampleRate,
                                            const unsigned int numSamples,
                                            benchmark::State* pState) {
    EffectManifest manifest = EffectType::getManifest();

    ChannelHandleFactory factory;
    QSet<ChannelHandleAndGroup> registeredChannels;

    QString channel1_group = QString("[Channel1]");
    ChannelHandle channel1 = factory.getOrCreateHandle(channel1_group);
    registeredChannels.insert(ChannelHandleAndGroup(channel1, channel1_group));
    EffectInstantiatorPointer pInstantiator = EffectInstantiatorPointer(
        new EffectProcessorInstantiator<EffectType>());
    EngineEffect effect(manifest, registeredChannels, pInstantiator);

    GroupFeatureState featureState;
    EffectProcessor::EnableState enableState = EffectProcessor::ENABLED;

    SampleBuffer input(numSamples);
    SampleBuffer output(numSamples);

    while (pState->KeepRunning()) {
        effect.process(channel1, input.data(), output.data(), numSamples,
                       sampleRate, enableState, featureState);
    }
}

#define FOR_COMMON_BUFFER_SIZES(bm) bm->Arg(32)->Arg(64)->Arg(128)->Arg(256)->Arg(512)->Arg(1024)->Arg(2048)->Arg(4096);


#define DECLARE_EFFECT_BENCHMARK(EffectName)                           \
static void BM_NativeEffects_DefaultParameters_##EffectName(           \
        benchmark::State& state) {                                     \
    ControlPotmeter loEqFrequency(                                     \
        ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);     \
    loEqFrequency.setDefaultValue(250.0);                              \
    ControlPotmeter hiEqFrequency(                                     \
        ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);     \
    hiEqFrequency.setDefaultValue(2500.0);                             \
    benchmarkNativeEffectDefaultParameters<EffectName>(                \
        44100, state.range_x(), &state);                               \
}                                                                      \
FOR_COMMON_BUFFER_SIZES(BENCHMARK(BM_NativeEffects_DefaultParameters_##EffectName));

DECLARE_EFFECT_BENCHMARK(Bessel4LVMixEQEffect)
DECLARE_EFFECT_BENCHMARK(Bessel8LVMixEQEffect)
DECLARE_EFFECT_BENCHMARK(BitCrusherEffect)
DECLARE_EFFECT_BENCHMARK(EchoEffect)
DECLARE_EFFECT_BENCHMARK(FilterEffect)
DECLARE_EFFECT_BENCHMARK(FlangerEffect)
DECLARE_EFFECT_BENCHMARK(GraphicEQEffect)
DECLARE_EFFECT_BENCHMARK(LinkwitzRiley8EQEffect)
DECLARE_EFFECT_BENCHMARK(MoogLadder4FilterEffect)
DECLARE_EFFECT_BENCHMARK(PhaserEffect)
DECLARE_EFFECT_BENCHMARK(ReverbEffect)

}  // namespace
