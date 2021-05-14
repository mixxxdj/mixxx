#pragma once

#include "sources/audiosource.h"

namespace mixxx {

class AudioSourceProxy : public AudioSource {
  public:
    explicit AudioSourceProxy(
            AudioSourcePointer&& pAudioSource)
            : AudioSourceProxy(
                  std::move(pAudioSource),
                  pAudioSource->getSignalInfo()) {
    }
    AudioSourceProxy(
            AudioSourcePointer&& pAudioSource,
            const audio::SignalInfo& signalInfo)
            : AudioSource(*pAudioSource, signalInfo),
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
            const WritableSampleFrames& sampleFrames) override {
        DEBUG_ASSERT(getSignalInfo() == m_pAudioSource->getSignalInfo());
        DEBUG_ASSERT(getBitrate() == m_pAudioSource->getBitrate());
        DEBUG_ASSERT(frameIndexRange() == m_pAudioSource->frameIndexRange());
        return readSampleFramesClampedOn(*m_pAudioSource, sampleFrames);
    }

    void adjustFrameIndexRange(
            IndexRange frameIndexRange) final {
        // Ugly hack to keep both sources (inherited base + inner delegate) in sync!
        AudioSource::adjustFrameIndexRange(frameIndexRange);
        adjustFrameIndexRangeOn(*m_pAudioSource, frameIndexRange);
    }

    const AudioSourcePointer m_pAudioSource;
};

} // namespace mixxx
