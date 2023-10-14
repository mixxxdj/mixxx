#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include <QString>

/// Manages instantiation of an audio unit. Only for internal use.
class AudioUnitManager {
  public:
    AudioUnitManager(AVAudioUnitComponent* _Nullable component);
    ~AudioUnitManager();

    AudioUnitManager(const AudioUnitManager&) = delete;
    AudioUnitManager& operator=(const AudioUnitManager&) = delete;

    /// Fetches the audio unit if already instantiated.
    /// Non-blocking and thread-safe.
    AudioUnit _Nullable getAudioUnit();

  private:
    QString m_name;
    std::atomic<bool> m_isInstantiated;
    AudioUnit _Nullable m_audioUnit;

    void instantiateAudioUnitAsync(AVAudioUnitComponent* _Nullable component);
};
