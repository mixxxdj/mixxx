#pragma once

#include "sources/audiosourceproxy.h"
#include "track/track_decl.h"

namespace mixxx {

class TrackPointerHolder {
  public:
    explicit TrackPointerHolder(
            TrackPointer&& pTrack)
            : m_pTrack(std::move(pTrack)) {
    }

  private:
    TrackPointer m_pTrack;
};

// Keeps the Track object alive while accessing the audio data
// of the track. The Track object must not be deleted while
// accessing the corresponding file to avoid file
// corruption when writing metadata while the file
// is still in use.
class AudioSourceTrackProxy : private TrackPointerHolder,
                              // The audio source must be closed BEFORE releasing the track
                              // pointer to close any open file handles. Otherwise exporting
                              // track metadata into the same file may not work, because the
                              // file is still locked by the OS!
                              public AudioSourceProxy {
  public:
    static AudioSourcePointer create(
            TrackPointer pTrack,
            AudioSourcePointer pAudioSource) {
        return std::make_shared<AudioSourceTrackProxy>(
                std::move(pTrack),
                std::move(pAudioSource));
    }

    AudioSourceTrackProxy(
            TrackPointer pTrack,
            AudioSourcePointer pAudioSource)
            : TrackPointerHolder(std::move(pTrack)),
              AudioSourceProxy(std::move(pAudioSource)) {
    }
};

} // namespace mixxx
