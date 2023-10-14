#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioBaseTypes.h>

#include <QMutex>
#include <QtGlobal>

#include "effects/backends/au/aueffectprocessor.h"

AudioUnitManager::AudioUnitManager(AVAudioUnitComponent* component) {
    instantiateAudioUnitAsync(component);
}

AudioUnitManager::~AudioUnitManager() {
    if (m_isInstantiated.load()) {
        [m_audioUnit deallocateRenderResources];
    }
}

AUAudioUnit* _Nullable AudioUnitManager::getAudioUnit() {
    // We need to load this atomic flag to ensure that we don't get a partial
    // read of the audio unit pointer (probably extremely uncommon, but not
    // impossible: https://belkadan.com/blog/2023/10/Implicity-Atomic)
    if (!m_isInstantiated.load()) {
        return nil;
    }
    return m_audioUnit;
}

void AudioUnitManager::instantiateAudioUnitAsync(
        AVAudioUnitComponent* _Nullable component) {
    // NOTE: The component can be null if the lookup failed in
    // `AUBackend::createProcessor`, in which case the effect simply acts as an
    // identity function on the audio. Same applies when `AUEffectProcessor` is
    // default-initialized.
    if (!component) {
        return;
    }

    auto description = [component audioComponentDescription];
    auto options = kAudioComponentInstantiation_LoadOutOfProcess;

    // Instantiate the audio unit asynchronously.

    // NOTE: We don't need AVAudioUnit here, since we want to render samples
    // without using an `AUGraph` or `AVAudioEngine`. Hence the recommended
    // approach is to simply instantiate the `AUAudioUnit` directly.  See
    // https://www.mail-archive.com/coreaudio-api@lists.apple.com/msg01084.html

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    [AUAudioUnit instantiateWithComponentDescription:description
                 options:options
                 completionHandler:^(AUAudioUnit* _Nullable audioUnit, NSError* _Nullable error) {
        if (error == nil) {
            VERIFY_OR_DEBUG_ASSERT(!m_isInstantiated.load()) {
                return;
            }

            NSError* error = nil;
            [audioUnit allocateRenderResourcesAndReturnError:&error];
            if (error != nil) {
                qWarning() << "Audio Unit failed to allocate render resources"
                        << QString::fromNSString([error localizedDescription]);
                return;
            }

            m_audioUnit = audioUnit;
            m_isInstantiated.store(true);
        } else {
            qWarning() << "Could not instantiate audio unit:"
                       << QString::fromNSString([error localizedDescription]);
        }
    }];
    // clang-format on
}

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

AUEffectProcessor::AUEffectProcessor(AVAudioUnitComponent* component)
        : m_component(component), m_manager(component) {
}

void AUEffectProcessor::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    // TODO
}

void AUEffectProcessor::processChannel(AUEffectGroupState* channelState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    AUAudioUnit* _Nullable audioUnit = m_manager.getAudioUnit();
    if (!audioUnit) {
        qWarning() << "Audio Unit not instantiated yet";
        return;
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
    [audioUnit renderBlock](&flags,
            &timestamp,
            framesToRender,
            outputBusNumber,
            &outputData,
            pullInput);
}
