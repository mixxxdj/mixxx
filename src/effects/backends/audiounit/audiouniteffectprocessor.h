#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <QList>
#include <atomic>

#include "audio/types.h"
#include "effects/backends/audiounit/audiounitmanager.h"
#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"

class AudioUnitEffectGroupState final : public EffectState {
  public:
    AudioUnitEffectGroupState(const mixxx::EngineParameters& engineParameters);

    void render(AudioUnit _Nonnull audioUnit,
            const mixxx::EngineParameters& engineParameters,
            const CSAMPLE* _Nonnull pInput,
            CSAMPLE* _Nonnull pOutput);

  private:
    AudioTimeStamp m_timestamp;
    AudioBufferList m_inputBuffers;
    AudioBufferList m_outputBuffers;

    static OSStatus renderCallbackUntyped(void* _Nonnull rawThis,
            AudioUnitRenderActionFlags* _Nonnull inActionFlags,
            const AudioTimeStamp* _Nonnull inTimeStamp,
            UInt32 inBusNumber,
            UInt32 inNumFrames,
            AudioBufferList* _Nonnull ioData);
    OSStatus renderCallback(AudioUnitRenderActionFlags* _Nonnull inActionFlags,
            const AudioTimeStamp* _Nonnull inTimeStamp,
            UInt32 inBusNumber,
            UInt32 inNumFrames,
            AudioBufferList* _Nonnull ioData) const;
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
    AudioUnitManagerPointer m_pManager;

    QList<EngineEffectParameterPointer> m_parameters;
    QList<AudioUnitParameterValue> m_lastValues;
    mixxx::audio::ChannelCount m_lastChannelCount;
    mixxx::audio::SampleRate m_lastSampleRate;

    void syncParameters();
    void syncStreamFormat(const mixxx::EngineParameters& engineParameters);
};
