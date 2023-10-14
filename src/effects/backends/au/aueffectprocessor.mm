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
    m_inputBuffers.mNumberBuffers = 1;
    m_outputBuffers.mNumberBuffers = 1;
}

// static
OSStatus AUEffectGroupState::renderCallbackUntyped(void* rawThis,
        AudioUnitRenderActionFlags* inActionFlags,
        const AudioTimeStamp* inTimeStamp,
        UInt32 inBusNumber,
        UInt32 inNumFrames,
        AudioBufferList* ioData) {
    return static_cast<AUEffectGroupState*>(rawThis)->renderCallback(
            inActionFlags, inTimeStamp, inBusNumber, inNumFrames, ioData);
}

OSStatus AUEffectGroupState::renderCallback(
        AudioUnitRenderActionFlags* inActionFlags,
        const AudioTimeStamp* inTimeStamp,
        UInt32 inBusNumber,
        UInt32 inNumFrames,
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

void AUEffectGroupState::render(AudioUnit _Nonnull audioUnit,
        const CSAMPLE* _Nonnull pInput,
        CSAMPLE* _Nonnull pOutput) {
    // Fill the input and output buffers.
    // TODO: Assert the size
    m_inputBuffers.mBuffers[0].mData = const_cast<CSAMPLE*>(pInput);
    m_outputBuffers.mBuffers[0].mData = pOutput;

    // Set the render callback
    AURenderCallbackStruct callback;
    callback.inputProc = AUEffectGroupState::renderCallbackUntyped;
    callback.inputProcRefCon = this;

    OSStatus setCallbackStatus = AudioUnitSetProperty(audioUnit,
            kAudioUnitProperty_SetRenderCallback,
            kAudioUnitScope_Input,
            0,
            &callback,
            sizeof(AURenderCallbackStruct));
    if (setCallbackStatus != noErr) {
        qWarning() << "Setting Audio Unit render callback failed with status"
                   << setCallbackStatus;
        return;
    }

    // Apply the actual effect to the sample.
    AudioUnitRenderActionFlags flags = 0;
    AUAudioFrameCount framesToRender = 1;
    NSInteger outputBusNumber = 0;
    OSStatus renderStatus = AudioUnitRender(audioUnit,
            &flags,
            &m_timestamp,
            outputBusNumber,
            framesToRender,
            &m_outputBuffers);
    if (renderStatus != noErr) {
        qWarning() << "Rendering Audio Unit failed with status" << renderStatus;
        return;
    }

    // Increment the timestamp
    m_timestamp.mSampleTime += framesToRender;
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
    AudioUnit _Nullable audioUnit = m_manager.getAudioUnit();
    if (!audioUnit) {
        qWarning() << "Audio Unit is not instantiated yet";
        return;
    }

    // TODO: Set format (even though Core Audio seems to default to 32-bit
    // floats, 2 channels and 44.1kHz sample rate)

    channelState->render(audioUnit, pInput, pOutput);
}
