#include "effects/backends/au/audiounitmanager.h"

AudioUnitManager::AudioUnitManager(AVAudioUnitComponent* _Nullable component) {
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
