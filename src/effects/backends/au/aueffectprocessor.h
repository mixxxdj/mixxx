#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <atomic>

#include "effects/backends/effectprocessor.h"

/// Manages instantiation of an audio unit. Only for internal use.
class AudioUnitManager {
  public:
    AudioUnitManager(AVAudioUnitComponent* component);
    ~AudioUnitManager();

    AudioUnitManager(const AudioUnitManager&) = delete;
    AudioUnitManager& operator=(const AudioUnitManager&) = delete;

    /// Fetches the audio unit if already instantiated.
    /// Non-blocking and thread-safe.
    AUAudioUnit* _Nullable getAudioUnit();

  private:
    std::atomic<bool> m_isInstantiated;
    AUAudioUnit* m_audioUnit;

    void instantiateAudioUnitAsync(AVAudioUnitComponent* _Nullable component);
};

class AUEffectGroupState final : public EffectState {
  public:
    AUEffectGroupState(const mixxx::EngineParameters& engineParameters);

    AudioTimeStamp getTimestamp();
    void incrementTimestamp();

  private:
    AudioTimeStamp m_timestamp;
};

class AUEffectProcessor final : public EffectProcessorImpl<AUEffectGroupState> {
  public:
    AUEffectProcessor(AVAudioUnitComponent* component = nil);

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
    AudioUnitManager m_manager;
};
