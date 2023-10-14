#pragma once

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

/// Manages instantiation of an audio unit. Only for internal use.
class AudioUnitManager {
  public:
    AudioUnitManager(AVAudioUnitComponent* _Nullable component);
    ~AudioUnitManager();

    AudioUnitManager(const AudioUnitManager&) = delete;
    AudioUnitManager& operator=(const AudioUnitManager&) = delete;

    /// Fetches the audio unit if already instantiated.
    /// Non-blocking and thread-safe.
    AUAudioUnit* _Nullable getAudioUnit();

  private:
    std::atomic<bool> m_isInstantiated;
    AUAudioUnit* _Nullable m_audioUnit;

    void instantiateAudioUnitAsync(AVAudioUnitComponent* _Nullable component);
};
