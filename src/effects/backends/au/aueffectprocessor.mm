#import <AVFAudio/AVFAudio.h>

#include <QMutex>
#include <QtGlobal>

#include "effects/backends/au/aueffectprocessor.h"

AUEffectGroupState::AUEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_isInstantiating(false),
          m_audioUnit(nil) {
}

AVAudioUnit* _Nullable AUEffectGroupState::getAudioUnit() {
    // We need to load this atomic flag to ensure that we don't get a partial
    // read of the audio unit pointer (probably extremely uncommon, but not
    // impossible: https://belkadan.com/blog/2023/10/Implicity-Atomic)
    if (!m_isInstantiated.load()) {
        return nil;
    }
    return m_audioUnit;
}

void AUEffectGroupState::instantiateAudioUnitAsyncIfNeeded(
        AVAudioUnitComponent* _Nullable component) {
    // Ensure that we only instantiate once
    if (!m_isInstantiating.exchange(true)) {
        instantiateAudioUnitAsync(component);
    }
}

void AUEffectGroupState::instantiateAudioUnitAsync(
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

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    [AVAudioUnit instantiateWithComponentDescription:description
                 options:options
                 completionHandler:^(AVAudioUnit* _Nullable audioUnit, NSError* _Nullable error) {
        if (error == nil) {
            VERIFY_OR_DEBUG_ASSERT(!m_isInstantiated.load()) {
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

AUEffectProcessor::AUEffectProcessor(AVAudioUnitComponent* component)
        : m_component(component) {
}

AUEffectGroupState* AUEffectProcessor::createSpecificState(
        const mixxx::EngineParameters& engineParameters) {
    AUEffectGroupState* state = new AUEffectGroupState(engineParameters);
    state->instantiateAudioUnitAsyncIfNeeded(m_component);
    return state;
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
    // TODO
}
