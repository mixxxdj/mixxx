#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <QSharedPointer>
#include <QString>

enum AudioUnitInstantiationType {
    Sync,
    AsyncInProcess,
    AsyncOutOfProcess,
};

class AudioUnitManager;
typedef QSharedPointer<AudioUnitManager> AudioUnitManagerPointer;

/// A RAII wrapper around an `AudioUnit`.
class AudioUnitManager {
  public:
    ~AudioUnitManager();

    AudioUnitManager(const AudioUnitManager&) = delete;
    AudioUnitManager& operator=(const AudioUnitManager&) = delete;

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

  private:
    QString m_name;
    std::atomic<bool> m_isInstantiated;
    AudioUnit _Nullable m_audioUnit;

    AudioUnitManager(AVAudioUnitComponent* _Nullable component);

    static void instantiateAudioUnitAsync(AudioUnitManagerPointer manager,
            AVAudioUnitComponent* _Nonnull component,
            bool inProcess);
    static void instantiateAudioUnitSync(AudioUnitManagerPointer manager,
            AVAudioUnitComponent* _Nonnull component);

    void initializeWith(AudioUnit _Nullable audioUnit);
};
