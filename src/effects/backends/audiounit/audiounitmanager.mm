#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#include "util/assert.h"

#include <QString>

#include "effects/backends/audiounit/audiounitmanager.h"

AudioUnitManager::AudioUnitManager(AVAudioUnitComponent* _Nullable component,
        AudioUnitInstantiationType instantiationType)
        : m_name(QString::fromNSString([component name])) {
    // NOTE: The component can be null if the lookup failed in
    // `AudioUnitBackend::createProcessor`, in which case the effect simply acts
    // as an identity function on the audio. Same applies when
    // `AudioUnitManager` is default-initialized.
    if (!component) {
        return;
    }

    switch (instantiationType) {
    case Sync:
        instantiateAudioUnitSync(component);
        break;
    case AsyncInProcess:
    case AsyncOutOfProcess:
        instantiateAudioUnitAsync(
                component, instantiationType == AsyncInProcess);
        break;
    }
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
        AVAudioUnitComponent* _Nonnull component, bool inProcess) {
    auto options = inProcess ? kAudioComponentInstantiation_LoadInProcess
                             : kAudioComponentInstantiation_LoadOutOfProcess;

    // Instantiate the audio unit asynchronously.
    qDebug() << "Instantiating Audio Unit" << m_name << "asynchronously";

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    AudioComponentInstantiate(component.audioComponent, options, ^(AudioUnit _Nullable audioUnit, OSStatus error) {
        if (error != noErr) {
            qWarning() << "Could not instantiate Audio Unit" << m_name << ":" << error << "(Check https://www.osstatus.com for a description)";
            return;
        }

        VERIFY_OR_DEBUG_ASSERT(audioUnit != nil) {
            qWarning() << "Could not instantiate Audio Unit" << m_name << "...but the error is noErr, what's going on?";
            return;
        }

        initializeWith(audioUnit);
    });
    // clang-format on
}

void AudioUnitManager::instantiateAudioUnitSync(
        AVAudioUnitComponent* _Nonnull component) {
    AudioUnit _Nullable audioUnit = nil;
    OSStatus error =
            AudioComponentInstanceNew(component.audioComponent, &audioUnit);
    if (error != noErr) {
        qWarning() << "Audio Unit" << m_name
                   << "could not be instantiated:" << error
                   << "(Check https://www.osstatus.com for a description)";
    }

    initializeWith(audioUnit);
}

void AudioUnitManager::initializeWith(AudioUnit _Nonnull audioUnit) {
    VERIFY_OR_DEBUG_ASSERT(!m_isInstantiated.load()) {
        qWarning() << "Audio Unit" << m_name
                   << "cannot be initialized after already having been "
                      "instantiated";
        return;
    }

    OSStatus initError = AudioUnitInitialize(audioUnit);
    if (initError != noErr) {
        qWarning() << "Audio Unit" << m_name
                   << "failed to initialize, i.e. allocate render resources:"
                   << initError
                   << "(Check https://www.osstatus.com for a description)";
        return;
    }

    m_audioUnit = audioUnit;
    m_isInstantiated.store(true);
}
