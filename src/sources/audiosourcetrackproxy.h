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
class AudioSourceTrackProxy: public AudioSource {
  public:
    static AudioSourcePointer create(
            AudioSourcePointer pAudioSource,
            TrackPointer pTrack) {
        return std::make_shared<AudioSourceTrackProxy>(
                std::move(pAudioSource),
                std::move(pTrack));
    }

    AudioSourceTrackProxy(
            AudioSourcePointer pAudioSource,
            TrackPointer pTrack)
        : AudioSource(*pAudioSource),
          m_pAudioSource(std::move(pAudioSource)),
          m_pTrack(std::move(pTrack)) {
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
    AudioSourcePointer m_pAudioSource;
    TrackPointer m_pTrack;
};

} // namespace mixxx


#endif // MIXXX_AUDIOSOURCETRACKPROXY_H
