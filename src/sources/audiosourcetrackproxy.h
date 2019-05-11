#ifndef MIXXX_AUDIOSOURCETRACKPROXY_H
#define MIXXX_AUDIOSOURCETRACKPROXY_H

#include "sources/audiosource.h"

#include "track/track.h"

namespace mixxx {

// Keeps the TIO alive while accessing the audio data
// of the track. The TIO must not be deleted while
// accessing the corresponding file to avoid file
// corruption when writing metadata while the file
// is still in use.
class AudioSourceTrackProxy : public AudioSource {
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
            : AudioSource(*pAudioSource),
              m_pTrack(std::move(pTrack)),
              m_pAudioSource(std::move(pAudioSource)) {
    }

    void close() override {
        m_pAudioSource->close();
    }

  protected:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override {
        return tryOpenOn(*m_pAudioSource, mode, params);
    }

    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override {
        return readSampleFramesClampedOn(*m_pAudioSource, sampleFrames);
    }

  private:
    TrackPointer m_pTrack;
    // The audio source must be closed before releasing the track
    // pointer to close any open file handles. Otherwise exporting
    // track metadata into the same file may not work, because the
    // file is still locked by the OS!
    AudioSourcePointer m_pAudioSource;
};

} // namespace mixxx

#endif // MIXXX_AUDIOSOURCETRACKPROXY_H
