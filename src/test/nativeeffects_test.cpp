#if 0
// TODO: make this work again
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
#include "test/baseeffecttest.h"
#include "util/samplebuffer.h"

namespace {

class EffectsBenchmarkTest : public BaseEffectTest {
  protected:
    void SetUp() override {
        registerTestBackend();
    }
};

template <class EffectType>
void benchmarkNativeEffectDefaultParameters(const mixxx::EngineParameters& bufferParameters,
                                            benchmark::State* pState, EffectsManager* pEffectsManager) {
    EffectManifest manifest = EffectType::getManifest();

    ChannelHandleFactory factory;
    QSet<ChannelHandleAndGroup> activeInputChannels;

    QString channel1_group = QString("[Channel1]");
    ChannelHandle channel1 = factory.getOrCreateHandle(channel1_group);
    ChannelHandleAndGroup handle_and_group(channel1, channel1_group);
    pEffectsManager->registerInputChannel(handle_and_group);
    pEffectsManager->registerOutputChannel(handle_and_group);
    activeInputChannels.insert(handle_and_group);
    EffectInstantiatorPointer pInstantiator = EffectInstantiatorPointer(
        new EffectProcessorInstantiator<EffectType>());
    EngineEffect effect(manifest, activeInputChannels, pEffectsManager, pInstantiator);

    GroupFeatureState featureState;
    EffectEnableState enableState = EffectEnableState::Enabled;

    mixxx::SampleBuffer input(bufferParameters.samplesPerBuffer());
    mixxx::SampleBuffer output(bufferParameters.samplesPerBuffer());

    while (pState->KeepRunning()) {
        effect.process(channel1, channel1, input.data(), output.data(),
                       bufferParameters.samplesPerBuffer(),
                       bufferParameters.sampleRate(),
                       enableState, featureState);
    }
}

#define FOR_COMMON_BUFFER_SIZES(bm) bm->Arg(32)->Arg(64)->Arg(128)->Arg(256)->Arg(512)->Arg(1024)->Arg(2048)->Arg(4096);

#define DECLARE_EFFECT_BENCHMARK(EffectName)                           \
TEST_F(EffectsBenchmarkTest, BM_NativeEffects_DefaultParameters_##EffectName) { \
    ControlPotmeter loEqFrequency(                                     \
        ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);     \
    loEqFrequency.setDefaultValue(250.0);                              \
    ControlPotmeter hiEqFrequency(                                     \
        ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);     \
    hiEqFrequency.setDefaultValue(2500.0);                             \
    mixxx::EngineParameters bufferParameters(                          \
        mixxx::AudioSignal::SampleRate(44100),                         \
        state.range_x());                                              \
    benchmarkNativeEffectDefaultParameters<EffectName>(                \
        bufferParameters, &state, m_pEffectsManager);                                     \
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
#endif

