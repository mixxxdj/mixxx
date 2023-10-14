#import <AudioToolbox/AudioToolbox.h>

#include "effects/backends/au/audiounitmanager.h"

AudioUnitManager::AudioUnitManager(AVAudioUnitComponent* _Nullable component) {
    instantiateAudioUnitAsync(component);
}

AudioUnitManager::~AudioUnitManager() {
    if (m_isInstantiated.load()) {
        AudioUnitUninitialize(m_audioUnit);
    }
}

AudioUnit _Nullable AudioUnitManager::getAudioUnit() {
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

    auto options = kAudioComponentInstantiation_LoadOutOfProcess;

    // Instantiate the audio unit asynchronously.

    // NOTE: We don't need AVAudioUnit here, since we want to render samples
    // without using an `AUGraph` or `AVAudioEngine`. Hence the recommended
    // approach is to simply instantiate the `AUAudioUnit` directly.  See
    // https://www.mail-archive.com/coreaudio-api@lists.apple.com/msg01084.html

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    AudioComponentInstantiate(component.audioComponent, options, ^(AudioUnit _Nullable audioUnit, OSStatus error) {
        if (error != noErr) {
            qWarning() << "Could not instantiate Audio Unit:" << error << "(Check https://www.osstatus.com for a description)";
            return;
        }

        VERIFY_OR_DEBUG_ASSERT(!m_isInstantiated.load()) {
            return;
        }

        OSStatus initError = AudioUnitInitialize(audioUnit);
        if (initError != noErr) {
            qWarning() << "Audio Unit failed to initialize, i.e. allocate render resources:" << initError << "(Check https://www.osstatus.com for a description)";
            return;
        }

        m_audioUnit = audioUnit;
        m_isInstantiated.store(true);
    });
    // clang-format on
}
