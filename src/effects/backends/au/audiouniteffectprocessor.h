#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <atomic>

#include "effects/backends/au/audiounitmanager.h"
#include "effects/backends/effectprocessor.h"

class AudioUnitEffectGroupState final : public EffectState {
  public:
    AudioUnitEffectGroupState(const mixxx::EngineParameters& engineParameters);

    void render(AudioUnit _Nonnull audioUnit,
            SINT sampleCount,
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

class AudioUnitEffectProcessor final : public EffectProcessorImpl<AudioUnitEffectGroupState> {
  public:
    AudioUnitEffectProcessor(AVAudioUnitComponent* _Nullable component = nil);

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters)
            override;

    void processChannel(AudioUnitEffectGroupState* _Nonnull channelState,
            const CSAMPLE* _Nonnull pInput,
            CSAMPLE* _Nonnull pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    std::atomic<bool> m_isConfigured;
    AudioUnitManager m_manager;
};
