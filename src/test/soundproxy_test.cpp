#include <QTemporaryFile>
#include <QtDebug>

#include "test/mixxxtest.h"

#include "sources/soundsourceproxy.h"
#include "sources/audiosourcestereoproxy.h"
#include "track/trackmetadata.h"
#include "util/samplebuffer.h"

#ifdef __OPUS__
#include "sources/soundsourceopus.h"
#endif // __OPUS__

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

const SINT kBufferSizes[] = {
        256,
        512,
        768,
        1024,
        1536,
        2048,
        3072,
        4096,
        6144,
        8192,
        12288,
        16384,
        24576,
        32768,
};

const SINT kMaxReadFrameCount = kBufferSizes[sizeof(kBufferSizes) / sizeof(kBufferSizes[0]) - 1];

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

        // All test files are mono, but we are requesting a stereo signal
        // to test the upscaling of channels
        mixxx::AudioSource::OpenParams openParams;
        openParams.setChannelCount(2);
        auto pAudioSource = proxy.openAudioSource(openParams);
        EXPECT_FALSE(!pAudioSource);
        if (pAudioSource->channelCount() != 2) {
            // Wrap into proxy object
            pAudioSource = mixxx::AudioSourceStereoProxy::create(
                    pAudioSource,
                    kMaxReadFrameCount);
        }
        EXPECT_EQ(pAudioSource->channelCount(), 2);
        return pAudioSource;
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

    mixxx::IndexRange skipSampleFrames(
            mixxx::AudioSourcePointer pAudioSource,
            mixxx::IndexRange skipRange) {
        mixxx::IndexRange skippedRange;
        while (skippedRange.length() < skipRange.length()) {
            // Seek in forward direction by decoding and discarding samples
            const auto nextRange =
                    mixxx::IndexRange::forward(
                            skippedRange.empty() ? skipRange.start() : skippedRange.end(),
                            math_min(
                                    skipRange.length() - skippedRange.length(),
                                    pAudioSource->samples2frames(m_skipSampleBuffer.size())));
            EXPECT_FALSE(nextRange.empty());
            EXPECT_TRUE(intersect(nextRange, skipRange) == nextRange);
            const auto readRange = pAudioSource->readSampleFrames(
                    mixxx::WritableSampleFrames(
                            nextRange,
                            mixxx::SampleBuffer::WritableSlice(
                                    m_skipSampleBuffer.data(),
                                    m_skipSampleBuffer.size()))).frameIndexRange();
            if (readRange.empty()) {
                return skippedRange;
            }
            EXPECT_TRUE(readRange.start() == nextRange.start());
            EXPECT_TRUE(intersect(readRange, skipRange) == readRange);
            if (skippedRange.empty()) {
                skippedRange = readRange;
            } else {
                skippedRange = span(skippedRange, nextRange);
            }
        }
        return skippedRange;
    }

    SoundSourceProxyTest()
        : m_skipSampleBuffer(kMaxReadFrameCount) {
    }

  private:
    mixxx::SampleBuffer m_skipSampleBuffer;
};

TEST_F(SoundSourceProxyTest, open) {
    // This test piggy-backs off of the cover-test files.
    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        mixxx::AudioSourcePointer pAudioSource = openAudioSource(filePath);
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pAudioSource) {
            // skip test file
            continue;
        }
        EXPECT_LT(0, pAudioSource->channelCount());
        EXPECT_LT(0, pAudioSource->sampleRate());
        EXPECT_FALSE(pAudioSource->frameIndexRange().empty());
    }
}

TEST_F(SoundSourceProxyTest, openEmptyFile) {
    for (const auto& fileNameSuffix: getFileNameSuffixes()) {
        QTemporaryFile tempFile("emptyXXXXXX" + fileNameSuffix);
        qDebug() << "Created testing to open empty file:"
                << tempFile.fileName();
        tempFile.open();
        tempFile.close();

        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(tempFile.fileName()));
        auto pTrack = Track::newTemporary(tempFile.fileName());
        SoundSourceProxy proxy(pTrack);

        auto pAudioSource = proxy.openAudioSource();
        EXPECT_TRUE(!pAudioSource);
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    auto pTrack = Track::newTemporary(
            kTestDir.absoluteFilePath("artist.mp3"));
    SoundSourceProxy proxy(pTrack);
    mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(mixxx::MetadataSource::ImportResult::Succeeded, proxy.importTrackMetadata(&trackMetadata));
    EXPECT_EQ("Test Artist", trackMetadata.getTrackInfo().getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    auto pTrack = Track::newTemporary(
            kTestDir.absoluteFilePath("TOAL_TPE2.mp3"));
    SoundSourceProxy proxy(pTrack);
    mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(mixxx::MetadataSource::ImportResult::Succeeded, proxy.importTrackMetadata(&trackMetadata));
    EXPECT_EQ("TITLE2", trackMetadata.getTrackInfo().getArtist());
    EXPECT_EQ("ARTIST", trackMetadata.getAlbumInfo().getTitle());
    EXPECT_EQ("TITLE", trackMetadata.getAlbumInfo().getArtist());
    // The COMM:iTunPGAP comment should not be read
    EXPECT_TRUE(trackMetadata.getTrackInfo().getComment().isNull());
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
        mixxx::SampleBuffer contReadData(
                pContReadSource->frames2samples(kReadFrameCount));
        mixxx::SampleBuffer seekReadData(
                pContReadSource->frames2samples(kReadFrameCount));

        SINT contFrameIndex = pContReadSource->frameIndexMin();
        while (pContReadSource->frameIndexRange().containsIndex(contFrameIndex)) {
            const auto readFrameIndexRange =
                    mixxx::IndexRange::forward(contFrameIndex, kReadFrameCount);
            qDebug() << "Seeking and reading" << readFrameIndexRange;

            // Read next chunk of frames for Cont source without seeking
            const auto contSampleFrames =
                    pContReadSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    readFrameIndexRange,
                                    mixxx::SampleBuffer::WritableSlice(contReadData)));
            ASSERT_FALSE(contSampleFrames.frameIndexRange().empty());
            ASSERT_LE(contSampleFrames.frameIndexRange(), readFrameIndexRange);
            ASSERT_EQ(contSampleFrames.frameIndexRange().start(), readFrameIndexRange.start());
            contFrameIndex += contSampleFrames.frameLength();

            const SINT sampleCount =
                    pContReadSource->frames2samples(contSampleFrames.frameLength());

            mixxx::AudioSourcePointer pSeekReadSource(openAudioSource(filePath));
            ASSERT_FALSE(!pSeekReadSource);
            ASSERT_EQ(pContReadSource->channelCount(), pSeekReadSource->channelCount());
            ASSERT_EQ(pContReadSource->frameIndexRange(), pSeekReadSource->frameIndexRange());

            // Seek source to next chunk and read it
            auto seekSampleFrames =
                    pSeekReadSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    readFrameIndexRange,
                                    mixxx::SampleBuffer::WritableSlice(seekReadData)));

            // Both buffers should be equal
            ASSERT_EQ(contSampleFrames.frameIndexRange(), seekSampleFrames.frameIndexRange());
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
            seekSampleFrames =
                    pSeekReadSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    readFrameIndexRange,
                                    mixxx::SampleBuffer::WritableSlice(seekReadData)));

            // Both buffers should again be equal
            ASSERT_EQ(contSampleFrames.frameIndexRange(), seekSampleFrames.frameIndexRange());
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
    for (auto kReadFrameCount: kBufferSizes) {
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

            mixxx::SampleBuffer contReadData(
                    pContReadSource->frames2samples(kReadFrameCount));
            mixxx::SampleBuffer skipReadData(
                    pSkipReadSource->frames2samples(kReadFrameCount));

            SINT minFrameIndex = pContReadSource->frameIndexMin();
            SINT skipCount = 1;
            while (pContReadSource->frameIndexRange().containsIndex(minFrameIndex += skipCount)) {
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
                    auto const skippedSampleFrames =
                            pContReadSource->readSampleFrames(
                                    mixxx::WritableSampleFrames(
                                            skippingFrameIndexRange,
                                            mixxx::SampleBuffer::WritableSlice(contReadData)));
                    ASSERT_FALSE(skippedSampleFrames.frameIndexRange().empty());
                    ASSERT_EQ(skippedSampleFrames.frameIndexRange().start(), contFrameIndex);
                    contFrameIndex += skippedSampleFrames.frameLength();
                }
                ASSERT_EQ(minFrameIndex, contFrameIndex);
                const auto contSampleFrames =
                        pContReadSource->readSampleFrames(
                                mixxx::WritableSampleFrames(
                                        readFrameIndexRange,
                                        mixxx::SampleBuffer::WritableSlice(contReadData)));
                ASSERT_FALSE(contSampleFrames.frameIndexRange().empty());
                ASSERT_LE(contSampleFrames.frameIndexRange(), readFrameIndexRange);
                ASSERT_EQ(contSampleFrames.frameIndexRange().start(), readFrameIndexRange.start());
                contFrameIndex += contSampleFrames.frameLength();

                const SINT sampleCount =
                        pContReadSource->frames2samples(contSampleFrames.frameLength());

                // Skip until reaching the frame index and read next chunk
                ASSERT_LE(skipFrameIndex, minFrameIndex);
                while (skipFrameIndex < minFrameIndex) {
                    auto const skippedFrameIndexRange =
                            skipSampleFrames(pSkipReadSource,
                                    mixxx::IndexRange::between(skipFrameIndex, minFrameIndex));
                    ASSERT_FALSE(skippedFrameIndexRange.empty());
                    ASSERT_EQ(skippedFrameIndexRange.start(), skipFrameIndex);
                    skipFrameIndex += skippedFrameIndexRange.length();
                }
                ASSERT_EQ(minFrameIndex, skipFrameIndex);
                const auto skippedSampleFrames =
                        pSkipReadSource->readSampleFrames(
                                mixxx::WritableSampleFrames(
                                        readFrameIndexRange,
                                        mixxx::SampleBuffer::WritableSlice(skipReadData)));

                skipFrameIndex += skippedSampleFrames.frameLength();

                // Both buffers should be equal
                ASSERT_EQ(contSampleFrames.frameIndexRange(), skippedSampleFrames.frameIndexRange());
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
        mixxx::SampleBuffer seekReadData(
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
                pSeekReadSource->frameLength() / 2);
        // ...and to the boundaries again in opposite order...
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMax());
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMin() + 1);
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMax() - 1);
        seekFrameIndices.push_back(pSeekReadSource->frameIndexMin());
        // ...near the end and back to middle of the stream...
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMax()
                - 4 * kReadFrameCount);
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMin()
                + pSeekReadSource->frameLength() / 2);
        // ...before the middle and then near the end of the stream...
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMin()
                + pSeekReadSource->frameLength() / 2
                - 4 * kReadFrameCount);
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMax()
                - 4 * kReadFrameCount);
        // ...to the moddle of the stream and then skipping kReadFrameCount samples.
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMin()
                + pSeekReadSource->frameLength() / 2);
        seekFrameIndices.push_back(
                pSeekReadSource->frameIndexMin()
                + pSeekReadSource->frameLength() / 2
                + 2 * kReadFrameCount);

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
                    skipSampleFrames(pContReadSource,
                            mixxx::IndexRange::between(
                                    pContReadSource->frameIndexMin(),
                                    seekFrameIndex));
            ASSERT_TRUE(skipFrameIndexRange.empty() ||
                    (skipFrameIndexRange.end() == seekFrameIndex));
            mixxx::SampleBuffer contReadData(
                    pContReadSource->frames2samples(kReadFrameCount));
            const auto contSampleFrames =
                    pContReadSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    readFrameIndexRange,
                                    mixxx::SampleBuffer::WritableSlice(contReadData)));
                    ASSERT_EQ(expectedFrameIndexRange, contSampleFrames.frameIndexRange());

            const auto seekSampleFrames =
                    pSeekReadSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    readFrameIndexRange,
                                    mixxx::SampleBuffer::WritableSlice(seekReadData)));
            ASSERT_EQ(expectedFrameIndexRange, seekSampleFrames.frameIndexRange());

            if (seekSampleFrames.frameIndexRange().empty()) {
                continue; // nothing to do
            }

            const SINT sampleCount =
                    pSeekReadSource->frames2samples(seekSampleFrames.frameLength());
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
        mixxx::SampleBuffer readBuffer(
                pAudioSource->frames2samples(kReadFrameCount));
        EXPECT_EQ(
                mixxx::IndexRange::forward(seekIndex, remainingFrames),
                pAudioSource->readSampleFrames(
                        mixxx::WritableSampleFrames(
                                mixxx::IndexRange::forward(seekIndex, kReadFrameCount),
                                mixxx::SampleBuffer::WritableSlice(readBuffer))).frameIndexRange());
    }
}

TEST_F(SoundSourceProxyTest, regressionTestCachingReaderChunkJumpForward) {
    // NOTE(uklotzde, 2017-12-10): Potential regression test for an infinite
    // seek/read loop in SoundSourceMediaFoundation. Unfortunately this
    // test doesn't fail even prior to fixing the reported bug.
    // https://github.com/mixxxdj/mixxx/pull/1317#issuecomment-349674161

    for (auto kReadFrameCount: kBufferSizes) {
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
            mixxx::SampleBuffer readBuffer(
                    pAudioSource->frames2samples(kReadFrameCount));

            // Read chunk from beginning
            auto firstChunkRange = mixxx::IndexRange::forward(
                    pAudioSource->frameIndexMin(), kReadFrameCount);
            EXPECT_EQ(
                    firstChunkRange,
                    pAudioSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    firstChunkRange,
                                    mixxx::SampleBuffer::WritableSlice(readBuffer))).frameIndexRange());

            // Read chunk from near the end, rounded to chunk boundary
            auto secondChunkRange = mixxx::IndexRange::forward(
                    ((pAudioSource->frameIndexMax() - 2 * kReadFrameCount) / kReadFrameCount) * kReadFrameCount, kReadFrameCount);
            EXPECT_EQ(
                    secondChunkRange,
                    pAudioSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    secondChunkRange,
                                    mixxx::SampleBuffer::WritableSlice(readBuffer))).frameIndexRange());
        }
    }
}
