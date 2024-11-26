#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <dispatch/dispatch.h>

#include <QString>

#include "effects/backends/audiounit/audiounitmanager.h"
#include "util/assert.h"

AudioUnitManager::AudioUnitManager(AVAudioUnitComponent* _Nullable component)
        : m_name(component != nil ? QString::fromNSString([component name])
                                  : "Unknown"),
          m_isInstantiated(false),
          m_instantiationGroup(dispatch_group_create()) {
}

AudioUnitManagerPointer AudioUnitManager::create(
        AVAudioUnitComponent* _Nullable component,
        AudioUnitInstantiationType instantiationType) {
    AudioUnitManagerPointer pManager =
            QSharedPointer<AudioUnitManager>(new AudioUnitManager(component));

    // NOTE: The component can be null if the lookup failed in
    // `AudioUnitBackend::createProcessor`, in which case the effect simply acts
    // as an identity function on the audio. Same applies when
    // `AudioUnitManager` is default-initialized.
    if (component) {
        switch (instantiationType) {
        case Sync:
            instantiateAudioUnitSync(pManager, component);
            break;
        case AsyncInProcess:
        case AsyncOutOfProcess:
            instantiateAudioUnitAsync(
                    pManager, component, instantiationType == AsyncInProcess);
            break;
        }
    }

    return pManager;
}

AudioUnitManager::~AudioUnitManager() {
    if (m_isInstantiated.load()) {
        qDebug() << "Uninitializing and disposing of Audio Unit" << m_name;
        AudioUnitUninitialize(m_audioUnit);
        AudioComponentInstanceDispose(m_audioUnit);
    }
}

AudioUnit _Nullable AudioUnitManager::getAudioUnit() const {
    // We need to load this atomic flag to ensure that we don't get a partial
    // read of the audio unit pointer (probably extremely uncommon, but not
    // impossible: https://belkadan.com/blog/2023/10/Implicity-Atomic)
    if (!m_isInstantiated.load()) {
        return nil;
    }
    return m_audioUnit;
}

bool AudioUnitManager::waitForAudioUnit(int timeoutMs) const {
    bool success =
            dispatch_group_wait(m_instantiationGroup,
                    dispatch_time(DISPATCH_TIME_NOW, timeoutMs * 1000000)) == 0;
    DEBUG_ASSERT(!success || m_isInstantiated.load());
    return success;
}

void AudioUnitManager::instantiateAudioUnitAsync(
        AudioUnitManagerPointer pManager,
        AVAudioUnitComponent* _Nonnull component,
        bool inProcess) {
    auto options = kAudioComponentInstantiation_LoadOutOfProcess;

    if (inProcess) {
#ifdef Q_OS_IOS
        qWarning() << "In-process Audio Unit instantiation is unavailable on "
                      "iOS, using out-of-process instantiation instead";
#else
        options = kAudioComponentInstantiation_LoadInProcess;
#endif
    }

    // Instantiate the audio unit asynchronously.
    qDebug() << "Instantiating Audio Unit" << pManager->m_name
             << "asynchronously";

    dispatch_group_enter(pManager->m_instantiationGroup);

    // TODO: Fix the weird formatting of blocks
    // clang-format off
    AudioComponentInstantiate(component.audioComponent, options, ^(AudioUnit _Nullable audioUnit, OSStatus error) {
        if (error != noErr) {
            qWarning() << "Could not instantiate Audio Unit"
                       << pManager->m_name << ":" << error
                       << "(Check https://www.osstatus.com for a description)";
            return;
        }

        pManager->initializeWith(audioUnit);
        dispatch_group_leave(pManager->m_instantiationGroup);
    });
    // clang-format on
}

void AudioUnitManager::instantiateAudioUnitSync(
        AudioUnitManagerPointer pManager,
        AVAudioUnitComponent* _Nonnull component) {
    AudioUnit _Nullable audioUnit = nil;
    OSStatus error =
            AudioComponentInstanceNew(component.audioComponent, &audioUnit);
    if (error != noErr) {
        qWarning() << "Audio Unit" << pManager->m_name
                   << "could not be instantiated:" << error
                   << "(Check https://www.osstatus.com for a description)";
    }

    pManager->initializeWith(audioUnit);
}

void AudioUnitManager::initializeWith(AudioUnit _Nullable audioUnit) {
    VERIFY_OR_DEBUG_ASSERT(audioUnit != nil) {
        qWarning() << "Instantiated Audio Unit" << m_name
                   << " is null, despite not erroring on initialization, "
                      "something's wrong";
        return;
    }

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
