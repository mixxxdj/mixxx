#include <QtDebug>

#include "test/mixxxtest.h"

#include "sources/soundsourceproxy.h"
#include "track/trackmetadata.h"
#include "util/samplebuffer.h"

#ifdef __OPUS__
#include "sources/soundsourceopus.h"
#endif // __OPUS__

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

} // anonymous namespace

class SoundSourceProxyTest: public MixxxTest {
  protected:
    static QStringList getFileNameSuffixes() {
        QStringList availableFileNameSuffixes;
        availableFileNameSuffixes
                << ".aiff"
                << ".flac"
                << ".m4a"
                << "-png.mp3"
                << ".ogg"
                << ".opus"
                << ".wav"
                << ".wv";

        QStringList supportedFileNameSuffixes;
        for (const auto& fileNameSuffix: availableFileNameSuffixes) {
            // We need to check for the whole file name here!
            if (SoundSourceProxy::isFileNameSupported(fileNameSuffix)) {
                supportedFileNameSuffixes << fileNameSuffix;
            }
        }
        return supportedFileNameSuffixes;
    }

    static QStringList getFilePaths() {
        QStringList filePaths;
        for (const auto& fileNameSuffix: getFileNameSuffixes()) {
            filePaths.append(kTestDir.absoluteFilePath("cover-test" + fileNameSuffix));
        }
        return filePaths;
    }

    enum class OpenAudioSourceMode {
        Default,
        DisableFFmpeg,
    };

    static mixxx::AudioSourcePointer openAudioSource(const QString& filePath, OpenAudioSourceMode mode = OpenAudioSourceMode::Default) {
        auto pTrack = Track::newTemporary(filePath);
        SoundSourceProxy proxy(pTrack);

        // TODO(XXX): Fix SoundSourceFFmpeg to avoid this special case handling
        if ((mode == OpenAudioSourceMode::DisableFFmpeg) &&
                proxy.getSoundSourceProvider() &&
                (proxy.getSoundSourceProvider()->getName() == "FFmpeg")) {
            qWarning()
                    << "Disabling test for FFmpeg:"
                    << filePath;
            return mixxx::AudioSourcePointer();
        }

        return proxy.openAudioSource();
    }

    static void expectDecodedSamplesEqual(
            SINT size,
            const CSAMPLE* expected,
            const CSAMPLE* actual,
            const char* errorMessage) {
        for (SINT i = 0; i < size; ++i) {
            EXPECT_EQ(expected[i], actual[i]) << errorMessage;
        }
    }

#ifdef __OPUS__
    // Known issue: Decoding with libopus is not sample accurate
    static void expectDecodedSamplesEqualOpus(
            SINT size,
            const CSAMPLE* expected,
            const CSAMPLE* actual,
            const char* errorMessage) {
        for (SINT i = 0; i < size; ++i) {
            EXPECT_NEAR(expected[i], actual[i],
                    mixxx::SoundSourceOpus::kMaxDecodingError) << errorMessage;
        }
    }
#endif // __OPUS__
};

TEST_F(SoundSourceProxyTest, open) {
    // This test piggy-backs off of the cover-test files.
    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        mixxx::AudioSourcePointer pAudioSource(openAudioSource(filePath));
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pAudioSource) {
            // skip test file
            continue;
        }
        EXPECT_LT(0, pAudioSource->channelCount());
        EXPECT_LT(0, pAudioSource->samplingRate());
        EXPECT_FALSE(pAudioSource->frameIndexRange().empty());
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    auto pTrack = Track::newTemporary(
            kTestDir.absoluteFilePath("artist.mp3"));
    SoundSourceProxy proxy(pTrack);
    mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, proxy.parseTrackMetadata(&trackMetadata));
    EXPECT_EQ("Test Artist", trackMetadata.getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    auto pTrack = Track::newTemporary(
            kTestDir.absoluteFilePath("TOAL_TPE2.mp3"));
    SoundSourceProxy proxy(pTrack);
    mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, proxy.parseTrackMetadata(&trackMetadata));
    EXPECT_EQ("TITLE2", trackMetadata.getArtist());
    EXPECT_EQ("ARTIST", trackMetadata.getAlbum());
    EXPECT_EQ("TITLE", trackMetadata.getAlbumArtist());
}

TEST_F(SoundSourceProxyTest, seekForwardBackward) {
    const SINT kReadFrameCount = 10000;

    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        qDebug() << "Seek forward/backward test:" << filePath;

        mixxx::AudioSourcePointer pContReadSource(openAudioSource(filePath));
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pContReadSource) {
            // skip test file
            continue;
        }
        SampleBuffer contReadData(
                pContReadSource->frames2samples(kReadFrameCount));
        SampleBuffer seekReadData(
                pContReadSource->frames2samples(kReadFrameCount));

        SINT contFrameIndex = pContReadSource->frameIndexMin();
        while (pContReadSource->frameIndexRange().contains(contFrameIndex)) {
            const auto readFrameIndexRange =
                    mixxx::IndexRange::forward(contFrameIndex, kReadFrameCount);
            qDebug() << "Seeking and reading" << readFrameIndexRange;

            // Read next chunk of frames for Cont source without seeking
            const auto contFrameIndexRange =
                    pContReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(contReadData));
            ASSERT_FALSE(contFrameIndexRange.empty());
            ASSERT_LE(contFrameIndexRange, readFrameIndexRange);
            ASSERT_EQ(contFrameIndexRange.start(), readFrameIndexRange.start());
            contFrameIndex += contFrameIndexRange.length();

            const SINT sampleCount =
                    pContReadSource->frames2samples(contFrameIndexRange.length());

            mixxx::AudioSourcePointer pSeekReadSource(openAudioSource(filePath));
            ASSERT_FALSE(!pSeekReadSource);
            ASSERT_EQ(pContReadSource->channelCount(), pSeekReadSource->channelCount());
            ASSERT_EQ(pContReadSource->frameIndexRange(), pSeekReadSource->frameIndexRange());

            // Seek source to next chunk and read it
            auto seekFrameIndexRange =
                    pSeekReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(seekReadData));

            // Both buffers should be equal
            ASSERT_EQ(contFrameIndexRange, seekFrameIndexRange);
#ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking forward");
            } else {
#endif // __OPUS__
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking forward");
#ifdef __OPUS__
            }
#endif // __OPUS__

            // Seek backwards to beginning of chunk and read again
            seekFrameIndexRange =
                    pSeekReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(seekReadData));

            // Both buffers should again be equal
            ASSERT_EQ(contFrameIndexRange, seekFrameIndexRange);
#ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking backward");
            } else {
#endif // __OPUS__
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking backward");
#ifdef __OPUS__
            }
#endif // __OPUS__
        }
    }
}

TEST_F(SoundSourceProxyTest, skipAndRead) {
    const SINT kReadFrameCount = 1000;

    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        qDebug() << "Skip and read test:" << filePath;

        mixxx::AudioSourcePointer pContReadSource(openAudioSource(filePath));
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pContReadSource) {
            // skip test file
            continue;
        }
        SINT contFrameIndex = pContReadSource->frameIndexMin();

        mixxx::AudioSourcePointer pSkipReadSource(openAudioSource(filePath));
        ASSERT_FALSE(!pSkipReadSource);
        ASSERT_EQ(pContReadSource->channelCount(), pSkipReadSource->channelCount());
        ASSERT_EQ(pContReadSource->frameIndexRange(), pSkipReadSource->frameIndexRange());
        SINT skipFrameIndex = pSkipReadSource->frameIndexMin();

        SampleBuffer contReadData(
                pContReadSource->frames2samples(kReadFrameCount));
        SampleBuffer skipReadData(
                pSkipReadSource->frames2samples(kReadFrameCount));

        SINT minFrameIndex = pContReadSource->frameIndexMin();
        SINT skipCount = 1;
        while (pContReadSource->frameIndexRange().contains(minFrameIndex += skipCount)) {
            skipCount = minFrameIndex / 4 + 1; // for next iteration

            qDebug() << "Skipping to:" << minFrameIndex;

            const auto readFrameIndexRange =
                    mixxx::IndexRange::forward(minFrameIndex, kReadFrameCount);

            // Read (and discard samples) until reaching the desired frame index
            // and read next chunk
            ASSERT_LE(contFrameIndex, minFrameIndex);
            while (contFrameIndex < minFrameIndex) {
                auto skippingFrameIndexRange =
                        mixxx::IndexRange::forward(
                                contFrameIndex,
                                std::min(minFrameIndex - contFrameIndex, kReadFrameCount));
                auto const skippedFrameIndexRange =
                        pContReadSource->readSampleFrames(
                                skippingFrameIndexRange,
                                SampleBuffer::WritableSlice(contReadData));
                ASSERT_FALSE(skippedFrameIndexRange.empty());
                ASSERT_EQ(skippedFrameIndexRange.start(), contFrameIndex);
                contFrameIndex += skippedFrameIndexRange.length();
            }
            ASSERT_EQ(minFrameIndex, contFrameIndex);
            const auto contFrameIndexRange =
                    pContReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(contReadData));
            ASSERT_FALSE(contFrameIndexRange.empty());
            ASSERT_LE(contFrameIndexRange, readFrameIndexRange);
            ASSERT_EQ(contFrameIndexRange.start(), readFrameIndexRange.start());
            contFrameIndex += contFrameIndexRange.length();

            const SINT sampleCount =
                    pContReadSource->frames2samples(contFrameIndexRange.length());

            // Skip until reaching the frame index and read next chunk
            ASSERT_LE(skipFrameIndex, minFrameIndex);
            while (skipFrameIndex < minFrameIndex) {
                auto const skippedFrameIndexRange =
                        pSkipReadSource->skipSampleFrames(
                                mixxx::IndexRange::between(skipFrameIndex, minFrameIndex));
                ASSERT_FALSE(skippedFrameIndexRange.empty());
                ASSERT_EQ(skippedFrameIndexRange.start(), skipFrameIndex);
                skipFrameIndex += skippedFrameIndexRange.length();
            }
            ASSERT_EQ(minFrameIndex, skipFrameIndex);
            const auto skipFrameIndexRange =
                    pSkipReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(skipReadData));
            skipFrameIndex += skipFrameIndexRange.length();

            // Both buffers should be equal
            ASSERT_EQ(contFrameIndexRange, skipFrameIndexRange);
#ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        sampleCount,
                        &contReadData[0],
                        &skipReadData[0],
                        "Decoding mismatch after skipping");
            } else {
#endif // __OPUS__
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &skipReadData[0],
                        "Decoding mismatch after skipping");
#ifdef __OPUS__
            }
#endif // __OPUS__

            minFrameIndex = contFrameIndex;
        }
    }
}

TEST_F(SoundSourceProxyTest, seekBoundaries) {
    const SINT kReadFrameCount = 1000;
    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        qDebug() << "Seek boundaries test:" << filePath;

        // TODO(XXX): Fix SoundSourceFFmpeg and re-enable testing
        mixxx::AudioSourcePointer pSeekReadSource(openAudioSource(filePath, OpenAudioSourceMode::DisableFFmpeg));
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pSeekReadSource) {
            // skip test file
            continue;
        }
        SampleBuffer seekReadData(
                pSeekReadSource->frames2samples(kReadFrameCount));

        std::vector<SINT> seekFrameIndices;
        // Seek to boundaries (alternating)...
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMin());
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMax() - 1);
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMin() + 1);
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMax());
        // ...seek to middle of the stream...
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMin() +
                pSeekReadSource->frameIndexRange().length() / 2);
        // ...and to the boundaries again in opposite order.
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMax());
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMin() + 1);
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMax() - 1);
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMin());

        // Read and verify results
        for (SINT seekFrameIndex: seekFrameIndices) {
            const auto readFrameIndexRange =
                    mixxx::IndexRange::forward(seekFrameIndex, kReadFrameCount);
            qDebug() << "Reading and verifying" << readFrameIndexRange;

            const auto expectedFrameIndexRange = intersect(
                    readFrameIndexRange,
                    pSeekReadSource->frameIndexRange());

            mixxx::AudioSourcePointer pContReadSource(openAudioSource(filePath));
            ASSERT_FALSE(!pContReadSource);
            ASSERT_EQ(pSeekReadSource->channelCount(), pContReadSource->channelCount());
            ASSERT_EQ(pSeekReadSource->frameIndexRange(), pContReadSource->frameIndexRange());
            const auto skipFrameIndexRange =
                    pContReadSource->skipSampleFrames(
                            mixxx::IndexRange::between(
                                    pContReadSource->frameIndexMin(),
                                    seekFrameIndex));
            ASSERT_TRUE(skipFrameIndexRange.empty() ||
                    (skipFrameIndexRange.end() == seekFrameIndex));
            SampleBuffer contReadData(
                    pContReadSource->frames2samples(kReadFrameCount));
            const auto contFrameIndexRange =
                    pContReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(contReadData));
            ASSERT_EQ(expectedFrameIndexRange, contFrameIndexRange);

            const auto seekFrameIndexRange =
                    pSeekReadSource->readSampleFrames(
                            readFrameIndexRange,
                            SampleBuffer::WritableSlice(seekReadData));
            ASSERT_EQ(expectedFrameIndexRange, seekFrameIndexRange);

            if (seekFrameIndexRange.empty()) {
                continue; // nothing to do
            }

            const SINT sampleCount =
                    pSeekReadSource->frames2samples(seekFrameIndexRange.length());
    #ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking");
            } else {
    #endif // __OPUS__
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking");
    #ifdef __OPUS__
            }
    #endif // __OPUS__
        }
    }
}

TEST_F(SoundSourceProxyTest, readBeyondEnd) {
    const SINT kReadFrameCount = 1000;

    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        qDebug() << "read beyond end test:" << filePath;

        mixxx::AudioSourcePointer pAudioSource(openAudioSource(filePath));
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pAudioSource) {
            // skip test file
            continue;
        }

        // Seek to position near the end
        const SINT seekIndex = pAudioSource->frameIndexMax() - (kReadFrameCount / 2);
        const SINT remainingFrames = pAudioSource->frameIndexMax() - seekIndex;
        ASSERT_GT(remainingFrames, 0);
        ASSERT_LT(remainingFrames, kReadFrameCount);

        // Read beyond the end
        SampleBuffer readBuffer(
                pAudioSource->frames2samples(kReadFrameCount));
        EXPECT_EQ(
                mixxx::IndexRange::forward(seekIndex, remainingFrames),
                pAudioSource->readSampleFrames(
                        mixxx::IndexRange::forward(seekIndex, kReadFrameCount),
                        SampleBuffer::WritableSlice(readBuffer)));
    }
}
