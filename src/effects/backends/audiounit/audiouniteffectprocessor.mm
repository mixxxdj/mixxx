#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioBaseTypes.h>
#include <CoreAudioTypes/CoreAudioTypes.h>

#include <QMutex>
#include <QtGlobal>

#include <algorithm>
#include <limits>

#include "effects/backends/audiounit/audiouniteffectprocessor.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/engine.h"
#include "util/assert.h"
#include "util/fpclassify.h"

AudioUnitEffectGroupState::AudioUnitEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_timestamp{
                  .mSampleTime = 0,
                  .mFlags = kAudioTimeStampSampleTimeValid,
          } {
    m_inputBuffers.mNumberBuffers = 1;
    m_outputBuffers.mNumberBuffers = 1;
}

// static
OSStatus AudioUnitEffectGroupState::renderCallbackUntyped(
        void* _Nonnull rawThis,
        AudioUnitRenderActionFlags* _Nonnull inActionFlags,
        const AudioTimeStamp* _Nonnull inTimeStamp,
        UInt32 inBusNumber,
        UInt32 inNumFrames,
        AudioBufferList* _Nonnull ioData) {
    return static_cast<AudioUnitEffectGroupState*>(rawThis)->renderCallback(
            inActionFlags, inTimeStamp, inBusNumber, inNumFrames, ioData);
}

OSStatus AudioUnitEffectGroupState::renderCallback(AudioUnitRenderActionFlags*,
        const AudioTimeStamp*,
        UInt32,
        UInt32,
        AudioBufferList* ioData) const {
    if (ioData->mNumberBuffers == 0) {
        qWarning() << "Audio unit render callback failed, no buffers available "
                      "to write to.";
        return noErr;
    }
    VERIFY_OR_DEBUG_ASSERT(m_inputBuffers.mNumberBuffers > 0) {
        qWarning() << "Audio unit render callback failed, no buffers available "
                      "to read from.";
        return noErr;
    }
    ioData->mBuffers[0].mData = m_inputBuffers.mBuffers[0].mData;
    return noErr;
}

void AudioUnitEffectGroupState::render(AudioUnit _Nonnull audioUnit,
        const mixxx::EngineParameters& engineParameters,
        const CSAMPLE* _Nonnull pInput,
        CSAMPLE* _Nonnull pOutput) {
    // Find the max number of samples per buffer that the Audio Unit can handle.
    //
    // Note that (confusingly) the Apple API refers to this count as "frames"
    // even though empirical tests show that this has to be interpreted as a
    // sample count, otherwise rendering the Audio Unit will fail with status
    // -10874 = kAudioUnitErr_TooManyFramesToProcess.
    //
    // For documentation on this property, see
    // https://developer.apple.com/documentation/audiotoolbox/kaudiounitproperty_maximumframesperslice
    //
    // TODO: Should we cache this?
    UInt32 maxSamplesPerChunk = 0;
    UInt32 maxSamplesPerChunkSize = sizeof(UInt32);
    OSStatus maxSamplesPerSliceStatus = AudioUnitGetProperty(audioUnit,
            kAudioUnitProperty_MaximumFramesPerSlice,
            kAudioUnitScope_Global,
            0,
            &maxSamplesPerChunk,
            &maxSamplesPerChunkSize);
    if (maxSamplesPerSliceStatus != noErr) {
        qWarning() << "Fetching the maximum samples per slice for Audio Unit "
                      "failed "
                      "with status"
                   << maxSamplesPerSliceStatus;
        return;
    }

    UInt32 totalSamples = engineParameters.samplesPerBuffer();

    // Process the buffer in chunks
    for (UInt32 offset = 0; offset < totalSamples;
            offset += maxSamplesPerChunk) {
        // Compute the size of the current slice.
        UInt32 remainingSamples = totalSamples - offset;
        UInt32 chunkSamples = std::min(remainingSamples, maxSamplesPerChunk);
        UInt32 chunkSize = sizeof(CSAMPLE) * chunkSamples;

        // Fill the input and output buffers.
        m_inputBuffers.mBuffers[0].mData =
                const_cast<CSAMPLE*>(pInput) + offset;
        m_inputBuffers.mBuffers[0].mDataByteSize = chunkSize;
        m_outputBuffers.mBuffers[0].mData = pOutput + offset;
        m_outputBuffers.mBuffers[0].mDataByteSize = chunkSize;

        // Set the render callback
        AURenderCallbackStruct callback;
        callback.inputProc = AudioUnitEffectGroupState::renderCallbackUntyped;
        callback.inputProcRefCon = this;

        OSStatus setCallbackStatus = AudioUnitSetProperty(audioUnit,
                kAudioUnitProperty_SetRenderCallback,
                kAudioUnitScope_Input,
                0,
                &callback,
                sizeof(AURenderCallbackStruct));
        if (setCallbackStatus != noErr) {
            qWarning()
                    << "Setting Audio Unit render callback failed with status"
                    << setCallbackStatus;
            return;
        }

        // Apply the actual effect to the sample.
        AudioUnitRenderActionFlags flags = 0;
        NSInteger outputBusNumber = 0;
        OSStatus renderStatus = AudioUnitRender(audioUnit,
                &flags,
                &m_timestamp,
                outputBusNumber,
                chunkSamples,
                &m_outputBuffers);
        if (renderStatus != noErr) {
            qWarning() << "Rendering Audio Unit failed with status"
                       << renderStatus;
            return;
        }

        // Increment the timestamp
        m_timestamp.mSampleTime += chunkSamples;
    }
}

AudioUnitEffectProcessor::AudioUnitEffectProcessor(
        AVAudioUnitComponent* _Nullable component)
        : m_pManager(AudioUnitManager::create(component)) {
}

void AudioUnitEffectProcessor::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_parameters = parameters.values();
}

void AudioUnitEffectProcessor::processChannel(
        AudioUnitEffectGroupState* _Nonnull channelState,
        const CSAMPLE* _Nonnull pInput,
        CSAMPLE* _Nonnull pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState,
        const GroupFeatureState&) {
    AudioUnit _Nullable audioUnit = m_pManager->getAudioUnit();
    if (!audioUnit) {
        qWarning()
                << "Cannot process channel before Audio Unit is instantiated";
        return;
    }

    // Sync engine parameters with Audio Unit
    syncStreamFormat(engineParameters);

    // Sync effect parameters with Audio Unit
    syncParameters();

    // Render the effect into the output buffer
    channelState->render(audioUnit, engineParameters, pInput, pOutput);
}

void AudioUnitEffectProcessor::syncParameters() {
    AudioUnit _Nullable audioUnit = m_pManager->getAudioUnit();
    DEBUG_ASSERT(audioUnit != nil);

    m_lastValues.reserve(m_parameters.size());

    int i = 0;
    for (auto parameter : m_parameters) {
        if (m_lastValues.size() < i) {
            static_assert(sizeof(AudioUnitParameterValue) == sizeof(float));
            m_lastValues.push_back(util_float_nan());
        }
        DEBUG_ASSERT(m_lastValues.size() >= i);

        AudioUnitParameterID id = parameter->id().toInt();
        auto value = static_cast<AudioUnitParameterValue>(parameter->value());

        // Update parameter iff changed since the last sync
        if (m_lastValues[i] != value) {
            m_lastValues[i] = value;

            OSStatus status = AudioUnitSetParameter(
                    audioUnit, id, kAudioUnitScope_Global, 0, value, 0);
            if (status != noErr) {
                qWarning()
                        << "Could not set Audio Unit parameter" << id << ":"
                        << status
                        << "(Check https://www.osstatus.com for a description)";
            }
        }

        i++;
    }
}

void AudioUnitEffectProcessor::syncStreamFormat(
        const mixxx::EngineParameters& parameters) {
    AudioUnit _Nullable audioUnit = m_pManager->getAudioUnit();
    DEBUG_ASSERT(audioUnit != nil);

    if (parameters.sampleRate() != m_lastSampleRate ||
            parameters.channelCount() != m_lastChannelCount) {
        auto sampleRate = parameters.sampleRate();
        auto channelCount = parameters.channelCount();

        m_lastSampleRate = sampleRate;
        m_lastChannelCount = channelCount;

        AVAudioFormat* audioFormat = [[AVAudioFormat alloc]
                initWithCommonFormat:AVAudioPCMFormatFloat32
                          sampleRate:sampleRate
                            channels:channelCount
                         interleaved:false];

        const auto* streamFormat = [audioFormat streamDescription];

        qDebug() << "Updating Audio Unit stream format to sample rate"
                 << sampleRate << "and channel count" << channelCount;

        for (auto scope : {kAudioUnitScope_Input, kAudioUnitScope_Output}) {
            OSStatus status = AudioUnitSetProperty(audioUnit,
                    kAudioUnitProperty_StreamFormat,
                    scope,
                    0,
                    streamFormat,
                    sizeof(AudioStreamBasicDescription));

            if (status != noErr) {
                qWarning()
                        << "Could not set Audio Unit stream format to sample "
                           "rate"
                        << sampleRate << "and channel count" << channelCount
                        << ":" << status
                        << "(Check https://www.osstatus.com for a description)";
            }
        }
    }
}
