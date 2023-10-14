#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <atomic>

#include "effects/backends/au/audiounitmanager.h"
#include "effects/backends/effectprocessor.h"

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
    AUEffectProcessor(AVAudioUnitComponent* _Nullable component = nil);

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters)
            override;

    void processChannel(AUEffectGroupState* _Nonnull channelState,
            const CSAMPLE* _Nonnull pInput,
            CSAMPLE* _Nonnull pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    std::atomic<bool> m_isConfigured;
    AudioUnitManager m_manager;
};
