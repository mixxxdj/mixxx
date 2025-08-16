#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <QString>

#include "effects/backends/audiounit/audiounitmanagerpointer.h"

enum AudioUnitInstantiationType {
    Sync,
    AsyncInProcess,
    AsyncOutOfProcess,
};

/// A RAII wrapper around an `AudioUnit`.
class AudioUnitManager {
  public:
    ~AudioUnitManager();

    AudioUnitManager(const AudioUnitManager&) = delete;
    AudioUnitManager& operator=(const AudioUnitManager&) = delete;

    /// Creates a new `AudioUnitManager`, wrapped in a shared pointer. This
    /// form of instantiation is enforced, since asynchronously instantiated
    /// audio units may capture a pointer to the manager instance (that would be
    /// unsafe if the manager is deinitialized to early, since the callback
    /// would be left with a dangling pointer).
    static AudioUnitManagerPointer create(
            AVAudioUnitComponent* _Nullable component = nil,
            AudioUnitInstantiationType instantiationType =
                    AudioUnitInstantiationType::AsyncOutOfProcess);

    /// Fetches the audio unit if already instantiated.
    ///
    /// Non-blocking and thread-safe, since this method is intended to (also) be
    /// called in a real-time context, e.g. from an audio thread, where we don't
    /// want to e.g. block on a mutex.
    AudioUnit _Nullable getAudioUnit() const;

    /// Blocks until the audio unit has been instantiated.
    ///
    /// Returns true if the audio unit was instantiated successfully and false if
    /// the timeout was reached instead.
    bool waitForAudioUnit(int timeoutMs) const;

  private:
    QString m_name;
    std::atomic<bool> m_isInstantiated;
    dispatch_group_t _Nonnull m_instantiationGroup;
    AudioUnit _Nullable m_audioUnit;

    AudioUnitManager(AVAudioUnitComponent* _Nullable component);

    static void instantiateAudioUnitAsync(AudioUnitManagerPointer pManager,
            AVAudioUnitComponent* _Nonnull component,
            bool inProcess);
    static void instantiateAudioUnitSync(AudioUnitManagerPointer pManager,
            AVAudioUnitComponent* _Nonnull component);

    void initializeWith(AudioUnit _Nullable audioUnit);
};
