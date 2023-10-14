#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioBaseTypes.h>

#include <QMutex>
#include <QtGlobal>

#include "effects/backends/au/aueffectprocessor.h"
#include "engine/engine.h"
#include "util/assert.h"

AUEffectGroupState::AUEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_timestamp{
                  .mSampleTime = 0,
                  .mFlags = kAudioTimeStampSampleTimeValid,
          } {
}

AudioTimeStamp AUEffectGroupState::getTimestamp() {
    return m_timestamp;
}

void AUEffectGroupState::incrementTimestamp() {
    m_timestamp.mSampleTime += 1;
}

AUEffectProcessor::AUEffectProcessor(AVAudioUnitComponent* _Nullable component)
        : m_manager(component) {
}

void AUEffectProcessor::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    // TODO
}

void AUEffectProcessor::processChannel(
        AUEffectGroupState* _Nonnull channelState,
        const CSAMPLE* _Nonnull pInput,
        CSAMPLE* _Nonnull pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    AUAudioUnit* _Nullable audioUnit = m_manager.getAudioUnit();
    if (!audioUnit) {
        qWarning() << "Audio Unit not instantiated yet";
        return;
    }

    if (!m_isConfigured.exchange(true)) {
        qDebug() << "Configuring Audio Unit to use a sample rate of"
                 << engineParameters.sampleRate() << "and a channel count of"
                 << engineParameters.channelCount();

        qWarning() << "Inputs:" << [[audioUnit inputBusses] count];
        qWarning() << "Outputs:" << [[audioUnit outputBusses] count];

        for (AUAudioUnitBusArray* buses in
                @[ [audioUnit inputBusses], [audioUnit outputBusses] ]) {
            for (AUAudioUnitBus* bus in buses) {
                NSError* error = nil;

                auto format = [[AVAudioFormat alloc]
                        initWithCommonFormat:AVAudioPCMFormatFloat32
                                  sampleRate:engineParameters.sampleRate()
                                    channels:engineParameters.channelCount()
                                 interleaved:false];

                [bus setFormat:format error:&error];

                if (error != nil) {
                    qWarning() << "Could not set Audio Unit format:"
                               << QString::fromNSString(
                                          [error localizedDescription]);
                    return;
                }
            }
        }
    }

    AudioTimeStamp timestamp = channelState->getTimestamp();
    channelState->incrementTimestamp();

    AudioUnitRenderActionFlags flags = 0;
    AUAudioFrameCount framesToRender = 1;
    NSInteger outputBusNumber = 0;
    AudioBufferList outputData = {.mNumberBuffers = 1,
            .mBuffers = {{
                    .mNumberChannels = 1,
                    .mDataByteSize = sizeof(CSAMPLE),
                    .mData = pOutput,
            }}};

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    AURenderPullInputBlock pullInput = ^(
        AudioUnitRenderActionFlags *actionFlags,
        const AudioTimeStamp *timestamp,
        AUAudioFrameCount frameCount,
        NSInteger inputBusNumber,
        AudioBufferList *inputData
    ) {
        VERIFY_OR_DEBUG_ASSERT(inputData->mNumberBuffers >= 1) {
            qWarning() << "AURenderPullInputBlock received empty input data";
            return 1; // TODO: Proper error-code
        };

        AudioBuffer* buffer = &inputData->mBuffers[0];

        VERIFY_OR_DEBUG_ASSERT(buffer->mDataByteSize == sizeof(CSAMPLE)) {
            qWarning() << "AURenderPullInputBlock encountered a buffer of size" << buffer->mDataByteSize << "(rather than the expected" << sizeof(CSAMPLE) << "bytes)";
            return 1; // TODO: Proper error-code
        }

        CSAMPLE* data = static_cast<CSAMPLE*>(buffer->mData);
        *data = *pInput;

        return 0; // No error
    };
    // clang-format on

    // Apply the actual effect to the sample.
    // See
    // https://developer.apple.com/documentation/audiotoolbox/aurenderblock?language=objc
    AUAudioUnitStatus status = [audioUnit renderBlock](&flags,
            &timestamp,
            framesToRender,
            outputBusNumber,
            &outputData,
            pullInput);

    if (status != 0) {
        qWarning() << "Rendering Audio Unit failed with status" << status;
        return;
    }
}
