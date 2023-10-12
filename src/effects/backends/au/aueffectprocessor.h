#pragma once

#import <AVFAudio/AVFAudio.h>

#include <atomic>

#include "effects/backends/effectprocessor.h"

class AUEffectGroupState final : public EffectState {
  public:
    AUEffectGroupState(const mixxx::EngineParameters& engineParameters);

    /// Fetches the audio unit if already instantiated. Non-blocking and
    /// thread-safe.
    AVAudioUnit* _Nullable getAudioUnit();

    /// Initiates an asynchronous instantiation of the audio unit if not already
    /// in progress. Non-blocking and thread-safe.
    void instantiateAudioUnitAsyncIfNeeded(AVAudioUnitComponent* _Nullable component);

  private:
    std::atomic<bool> m_isInstantiating;
    std::atomic<bool> m_isInstantiated;
    AVAudioUnit* m_audioUnit;

    void instantiateAudioUnitAsync(AVAudioUnitComponent* _Nullable component);
};

class AUEffectProcessor final : public EffectProcessorImpl<AUEffectGroupState> {
  public:
    AUEffectProcessor(AVAudioUnitComponent* component = nil);

    AUEffectGroupState* createSpecificState(
            const mixxx::EngineParameters& engineParameters) override;

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters)
            override;

    void processChannel(AUEffectGroupState* channelState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    AVAudioUnitComponent* _Nullable m_component;
};
