#pragma once

#include <qtemporarydir.h>

#include <memory>

#include "analyzer/analyzerbeats.h"
#include "analyzer/analyzerthread.h"
#include "control/control.h"
#include "sources/audiosource.h"
#include "sources/audiosourcestereoproxy.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "track/track_decl.h"

class AnalyzerBenchmark {
  public:
    AnalyzerBenchmark();

    mixxx::AudioSourcePointer run(TrackPointer pTrack);

  private:
    std::unique_ptr<AnalyzerWithState> m_analyzer;
    mixxx::SampleBuffer m_sampleBuffer;
    QTemporaryDir testDataDir;
    UserSettingsPointer m_pConfig;
    bool m_valid;
};

class TrackWrapper : public Track {
  public:
    TrackWrapper(const QString& filePath)
            : Track(mixxx::FileAccess(mixxx::FileInfo(filePath))) {
    }
    bool trySetBeats(mixxx::BeatsPointer pBeats) override {
        m_result = pBeats;
        return true;
    }

    const mixxx::BeatsPointer& result() const {
        return m_result;
    }

  private:
    mixxx::BeatsPointer m_result;
};

AnalyzerBenchmark::AnalyzerBenchmark()
        : m_sampleBuffer(mixxx::kAnalysisSamplesPerChunk),
          m_valid(false) {
    QFile test_cfg(testDataDir.filePath("test.cfg"));
    if (!test_cfg.open(QIODevice::ReadWrite)) {
        qWarning() << "Failed to open THE test config file:" << testDataDir.filePath("test.cfg");
        return;
    }
    test_cfg.close();
    m_pConfig = UserSettingsPointer(new UserSettings(
            testDataDir.filePath("test.cfg")));
    ControlDoublePrivate::setUserConfig(m_pConfig);
    m_analyzer = std::make_unique<AnalyzerWithState>(
            std::make_unique<AnalyzerBeats>(m_pConfig, true));
    m_valid = true;
}

mixxx::AudioSourcePointer AnalyzerBenchmark::run(TrackPointer pTrack) {
    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(mixxx::kAnalysisMaxChannels);

    // Get the audio
    mixxx::AudioSourcePointer audioSource =
            SoundSourceProxy(pTrack).openAudioSource(openParams);
    if (!audioSource) {
        return {};
    }

    auto track = AnalyzerTrack(pTrack);

    // If we have a non-even multi channel audio source (mono or )
    if (audioSource->getSignalInfo().getChannelCount() % mixxx::kAnalysisChannels) {
        audioSource = std::make_shared<mixxx::AudioSourceStereoProxy>(
                audioSource,
                mixxx::kAnalysisFramesPerChunk);
    }

    if (!m_analyzer->initialize(
                track,
                audioSource->getSignalInfo().getSampleRate(),
                audioSource->getSignalInfo().getChannelCount(),
                audioSource->frameLength())) {
        return {};
    }

    mixxx::IndexRange remainingFrameRange = audioSource->frameIndexRange();
    while (!remainingFrameRange.empty()) {
        // Split the range for the next chunk from the remaining (= to-be-analyzed) frames
        auto chunkFrameRange =
                remainingFrameRange.splitAndShrinkFront(
                        math_min(mixxx::kAnalysisFramesPerChunk, remainingFrameRange.length()));
        DEBUG_ASSERT(!chunkFrameRange.empty());

        // Request the next chunk of audio data
        const auto readableSampleFrames =
                audioSource->readSampleFrames(
                        mixxx::WritableSampleFrames(
                                chunkFrameRange,
                                mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));
        // The returned range fits into the requested range
        DEBUG_ASSERT(readableSampleFrames.frameIndexRange().isSubrangeOf(chunkFrameRange));

        // Sometimes the duration of the audio source is inaccurate and adjusted
        // while reading. We need to adjust all frame ranges to reflect this new
        // situation by restoring all invariants and consistency requirements!

        // Shrink the original range of the current chunks to the actual available
        // range.
        chunkFrameRange = intersect(chunkFrameRange, audioSource->frameIndexRange());
        // The audio data that has just been read should still fit into the adjusted
        // chunk range.
        DEBUG_ASSERT(readableSampleFrames.frameIndexRange().isSubrangeOf(chunkFrameRange));

        // We also need to adjust the remaining frame range for the next requests.
        remainingFrameRange = intersect(remainingFrameRange, audioSource->frameIndexRange());
        // Currently the range will never grow, but lets also account for this case
        // that might become relevant in the future.
        VERIFY_OR_DEBUG_ASSERT(remainingFrameRange.empty() ||
                remainingFrameRange.end() == audioSource->frameIndexRange().end()) {
            if (chunkFrameRange.length() < mixxx::kAnalysisFramesPerChunk) {
                // If we have read an incomplete chunk while the range has grown
                // we need to discard the read results and re-read the current
                // chunk!

                remainingFrameRange.growFront(chunkFrameRange.length());
                continue;
            }
            DEBUG_ASSERT(remainingFrameRange.end() < audioSource->frameIndexRange().end());
            remainingFrameRange.growBack(
                    audioSource->frameIndexRange().end() - remainingFrameRange.end());
        }

        // 2nd: step: Analyze chunk of decoded audio data
        if (!readableSampleFrames.frameIndexRange().empty()) {
            m_analyzer->processSamples(
                    readableSampleFrames.readableData(),
                    readableSampleFrames.readableLength());
        }

        // Don't check again for paused/stopped again and simply finish
        // the current iteration by emitting progress.

        // 3rd step: Update & emit progress
        if (audioSource->frameLength() > 0) {
            const double frameProgress =
                    static_cast<double>(audioSource->frameLength() - remainingFrameRange.length()) /
                    audioSource->frameLength();
            // math_min is required to compensate rounding errors
            const AnalyzerProgress progress =
                    math_min(kAnalyzerProgressFinalizing,
                            frameProgress *
                                    (kAnalyzerProgressFinalizing - kAnalyzerProgressNone));
            DEBUG_ASSERT(progress > kAnalyzerProgressNone);
        } else {
            // Unreadable audio source
            DEBUG_ASSERT(remainingFrameRange.empty());
        }
    }

    m_analyzer->finish(track);

    return audioSource;
}
