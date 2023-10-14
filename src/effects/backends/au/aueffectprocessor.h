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

    void render(AudioUnit _Nonnull audioUnit,
            const CSAMPLE* _Nonnull pInput,
            CSAMPLE* _Nonnull pOutput);

  private:
    AudioTimeStamp m_timestamp;
    AudioBufferList m_inputBuffers;
    AudioBufferList m_outputBuffers;

    static OSStatus renderCallbackUntyped(void* rawThis,
            AudioUnitRenderActionFlags* inActionFlags,
            const AudioTimeStamp* inTimeStamp,
            UInt32 inBusNumber,
            UInt32 inNumFrames,
            AudioBufferList* ioData);
    OSStatus renderCallback(AudioUnitRenderActionFlags* inActionFlags,
            const AudioTimeStamp* inTimeStamp,
            UInt32 inBusNumber,
            UInt32 inNumFrames,
            AudioBufferList* ioData) const;
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
