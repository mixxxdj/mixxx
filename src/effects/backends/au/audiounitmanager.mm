#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>

#include <QString>

#include "effects/backends/au/audiounitmanager.h"

AudioUnitManager::AudioUnitManager(AVAudioUnitComponent* _Nullable component)
        : m_name(QString::fromNSString([component name])) {
    instantiateAudioUnitAsync(component);
}

AudioUnitManager::~AudioUnitManager() {
    if (m_isInstantiated.load()) {
        qDebug() << "Uninitializing and disposing of Audio Unit" << m_name;
        AudioUnitUninitialize(m_audioUnit);
        AudioComponentInstanceDispose(m_audioUnit);
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
    qDebug() << "Instantiating Audio Unit" << m_name << "asynchronously";

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    AudioComponentInstantiate(component.audioComponent, options, ^(AudioUnit _Nullable audioUnit, OSStatus error) {
        if (error != noErr) {
            qWarning() << "Could not instantiate Audio Unit" << m_name << ":" << error << "(Check https://www.osstatus.com for a description)";
            return;
        }

        VERIFY_OR_DEBUG_ASSERT(!m_isInstantiated.load()) {
            return;
        }

        OSStatus initError = AudioUnitInitialize(audioUnit);
        if (initError != noErr) {
            qWarning() << "Audio Unit" << m_name << "failed to initialize, i.e. allocate render resources:" << initError << "(Check https://www.osstatus.com for a description)";
            return;
        }

        m_audioUnit = audioUnit;
        m_isInstantiated.store(true);
    });
    // clang-format on
}
