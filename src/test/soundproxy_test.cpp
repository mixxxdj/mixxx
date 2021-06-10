#include <QTemporaryFile>
#include <QtDebug>

#include "sources/audiosourcestereoproxy.h"
#include "sources/soundsourceproxy.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/track.h"
#include "track/trackmetadata.h"
#include "util/samplebuffer.h"

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

const CSAMPLE kMaxDecodingError = 0.01f;

} // anonymous namespace

class SoundSourceProxyTest : public MixxxTest, SoundSourceProviderRegistration {
  protected:
    static QStringList getFileNameSuffixes() {
        QStringList availableFileNameSuffixes;
        availableFileNameSuffixes
                << ".aiff"
                << "-alac.caf"
                << ".flac"
                // Files encoded with iTunes 12.3.0 caused issues when
                // decoding with FFMpeg 3.x, because their start_time
                // was not correctly handled. The actual FFmpeg version
                // that fixed this bug is unknown.
                << "-itunes-12.3.0-aac.m4a"
                << "-itunes-12.7.0-aac.m4a"
                << "-ffmpeg-aac.m4a"
#if defined(__FFMPEG__) || defined(__COREAUDIO__)
                << "-itunes-12.7.0-alac.m4a"
#endif
                << "-png.mp3"
                << "-vbr.mp3"
                << ".ogg"
                << ".opus"
                << ".wav"
                << ".wma"
                << ".wv";

        QStringList supportedFileNameSuffixes;
        for (const auto& fileNameSuffix : qAsConst(availableFileNameSuffixes)) {
            // We need to check for the whole file name here!
            if (SoundSourceProxy::isFileNameSupported(fileNameSuffix)) {
                supportedFileNameSuffixes << fileNameSuffix;
            } else {
                qInfo()
                        << "Ignoring unsupported file type"
                        << fileNameSuffix;
            }
        }
        return supportedFileNameSuffixes;
    }

    static QStringList getFilePaths() {
        QStringList filePaths;
        const QStringList fileNameSuffixes = getFileNameSuffixes();
        for (const auto& fileNameSuffix : fileNameSuffixes) {
            filePaths.append(kTestDir.absoluteFilePath("cover-test" + fileNameSuffix));
        }
        return filePaths;
    }

    static mixxx::AudioSourcePointer openAudioSource(
            const QString& filePath,
            const mixxx::SoundSourceProviderPointer& pProvider = nullptr) {
        auto pTrack = Track::newTemporary(filePath);
        SoundSourceProxy proxy(pTrack, pProvider);

        // All test files are mono, but we are requesting a stereo signal
        // to test the upscaling of channels
        mixxx::AudioSource::OpenParams openParams;
        const auto channelCount = mixxx::audio::ChannelCount(2);
        openParams.setChannelCount(mixxx::audio::ChannelCount(2));
        auto pAudioSource = proxy.openAudioSource(openParams);
        if (pAudioSource) {
            if (pAudioSource->getSignalInfo().getChannelCount() != channelCount) {
                // Wrap into proxy object
                pAudioSource = mixxx::AudioSourceStereoProxy::create(
                        pAudioSource,
                        kMaxReadFrameCount);
            }
            EXPECT_EQ(pAudioSource->getSignalInfo().getChannelCount(), channelCount);
            qInfo()
                    << "Opened file" << filePath
                    << "using provider" << proxy.getProvider()->getDisplayName();
        }
        return pAudioSource;
    }

    static void expectDecodedSamplesEqual(
            SINT size,
            const CSAMPLE* expected,
            const CSAMPLE* actual,
            const char* errorMessage) {
        for (SINT i = 0; i < size; ++i) {
            EXPECT_NEAR(expected[i], actual[i],
                    kMaxDecodingError) << errorMessage;
        }
    }

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
                                    pAudioSource->getSignalInfo().samples2frames(m_skipSampleBuffer.size())));
            EXPECT_FALSE(nextRange.empty());
            EXPECT_TRUE(nextRange.isSubrangeOf(skipRange));
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
            EXPECT_TRUE(readRange.isSubrangeOf(skipRange));
            if (skippedRange.empty()) {
                skippedRange = readRange;
            } else {
                EXPECT_TRUE(skippedRange.end() == nextRange.start());
                skippedRange.growBack(nextRange.length());
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
    const QStringList filePaths = getFilePaths();
    for (const auto& filePath : filePaths) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));
        const auto fileUrl = QUrl::fromLocalFile(filePath);
        const auto providerRegistrations =
                SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
        for (const auto& providerRegistration : providerRegistrations) {
            mixxx::AudioSourcePointer pAudioSource = openAudioSource(
                    filePath,
                    providerRegistration.getProvider());
            // Obtaining an AudioSource may fail for unsupported file formats,
            // even if the corresponding file extension is supported, e.g.
            // AAC vs. ALAC in .m4a files
            if (!pAudioSource) {
                // skip test file
                continue;
            }
            EXPECT_LT(0, pAudioSource->getSignalInfo().getChannelCount());
            EXPECT_LT(0, pAudioSource->getSignalInfo().getSampleRate());
            EXPECT_FALSE(pAudioSource->frameIndexRange().empty());
        }
    }
}

TEST_F(SoundSourceProxyTest, openEmptyFile) {
    const QStringList fileNameSuffixes = getFileNameSuffixes();
    for (const auto& fileNameSuffix : fileNameSuffixes) {
        const auto tmpFileName =
                mixxxtest::createEmptyTemporaryFile("emptyXXXXXX" + fileNameSuffix);
        const mixxxtest::FileRemover tmpFileRemover(tmpFileName);

        ASSERT_TRUE(QFile::exists(tmpFileName));
        ASSERT_TRUE(!tmpFileName.isEmpty());
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(tmpFileName));
        auto pTrack = Track::newTemporary(tmpFileName);
        SoundSourceProxy proxy(pTrack);

        auto pAudioSource = proxy.openAudioSource();
        EXPECT_TRUE(!pAudioSource);
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    auto pTrack = Track::newTemporary(kTestDir, "artist.mp3");
    SoundSourceProxy proxy(pTrack);
    EXPECT_TRUE(proxy.updateTrackFromSource());
    EXPECT_EQ("Test Artist", pTrack->getArtist());
}

TEST_F(SoundSourceProxyTest, readNoTitle) {
    // We need to verify every track has at least a title to not have empty lines in the library

    // Test a file with no metadata
    auto pTrack1 = Track::newTemporary(
            kTestDir, "empty.mp3");
    SoundSourceProxy proxy1(pTrack1);
    EXPECT_TRUE(proxy1.updateTrackFromSource());
    EXPECT_EQ("empty", pTrack1->getTitle());

    // Test a reload also works
    pTrack1->setTitle("");
    EXPECT_TRUE(proxy1.updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Again));
    EXPECT_EQ("empty", pTrack1->getTitle());

    // Test a file with other metadata but no title
    auto pTrack2 = Track::newTemporary(
            kTestDir, "cover-test-png.mp3");
    SoundSourceProxy proxy2(pTrack2);
    EXPECT_TRUE(proxy2.updateTrackFromSource());
    EXPECT_EQ("cover-test-png", pTrack2->getTitle());

    // Test a reload also works
    pTrack2->setTitle("");
    EXPECT_TRUE(proxy2.updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Again));
    EXPECT_EQ("cover-test-png", pTrack2->getTitle());

    // Test a file with a title
    auto pTrack3 = Track::newTemporary(
            kTestDir, "cover-test-jpg.mp3");
    SoundSourceProxy proxy3(pTrack3);
    EXPECT_TRUE(proxy3.updateTrackFromSource());
    EXPECT_EQ("test22kMono", pTrack3->getTitle());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    auto pTrack = Track::newTemporary(kTestDir, "TOAL_TPE2.mp3");
    SoundSourceProxy proxy(pTrack);
    mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(mixxx::MetadataSource::ImportResult::Succeeded,
            proxy.importTrackMetadataAndCoverImage(&trackMetadata, nullptr).first);
    EXPECT_EQ("TITLE2", trackMetadata.getTrackInfo().getArtist());
    EXPECT_EQ("ARTIST", trackMetadata.getAlbumInfo().getTitle());
    EXPECT_EQ("TITLE", trackMetadata.getAlbumInfo().getArtist());
    // The COMM:iTunPGAP comment should not be read
    EXPECT_TRUE(trackMetadata.getTrackInfo().getComment().isNull());
}

TEST_F(SoundSourceProxyTest, seekForwardBackward) {
    const SINT kReadFrameCount = 10000;

    const QStringList filePaths = getFilePaths();
    for (const auto& filePath : filePaths) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));
        qDebug() << "Seek forward/backward test:" << filePath;

        const auto fileUrl = QUrl::fromLocalFile(filePath);
        const auto providerRegistrations =
                SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
        for (const auto& providerRegistration : providerRegistrations) {
            mixxx::AudioSourcePointer pContReadSource = openAudioSource(
                    filePath,
                    providerRegistration.getProvider());

            // Obtaining an AudioSource may fail for unsupported file formats,
            // even if the corresponding file extension is supported, e.g.
            // AAC vs. ALAC in .m4a files
            if (!pContReadSource) {
                // skip test file
                continue;
            }
            mixxx::SampleBuffer contReadData(
                    pContReadSource->getSignalInfo().frames2samples(kReadFrameCount));
            mixxx::SampleBuffer seekReadData(
                    pContReadSource->getSignalInfo().frames2samples(kReadFrameCount));

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
                ASSERT_TRUE(contSampleFrames.frameIndexRange().isSubrangeOf(readFrameIndexRange));
                ASSERT_EQ(contSampleFrames.frameIndexRange().start(), readFrameIndexRange.start());
                contFrameIndex += contSampleFrames.frameLength();

                const SINT sampleCount =
                        pContReadSource->getSignalInfo().frames2samples(contSampleFrames.frameLength());

                mixxx::AudioSourcePointer pSeekReadSource = openAudioSource(
                        filePath,
                        providerRegistration.getProvider());

                ASSERT_FALSE(!pSeekReadSource);
                ASSERT_EQ(
                        pContReadSource->getSignalInfo().getChannelCount(),
                        pSeekReadSource->getSignalInfo().getChannelCount());
                ASSERT_EQ(pContReadSource->frameIndexRange(), pSeekReadSource->frameIndexRange());

                // Seek source to next chunk and read it
                auto seekSampleFrames =
                        pSeekReadSource->readSampleFrames(
                                mixxx::WritableSampleFrames(
                                        readFrameIndexRange,
                                        mixxx::SampleBuffer::WritableSlice(seekReadData)));

                // Both buffers should be equal
                ASSERT_EQ(contSampleFrames.frameIndexRange(), seekSampleFrames.frameIndexRange());
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking forward");

                // Seek backwards to beginning of chunk and read again
                seekSampleFrames =
                        pSeekReadSource->readSampleFrames(
                                mixxx::WritableSampleFrames(
                                        readFrameIndexRange,
                                        mixxx::SampleBuffer::WritableSlice(seekReadData)));

                // Both buffers should again be equal
                ASSERT_EQ(contSampleFrames.frameIndexRange(), seekSampleFrames.frameIndexRange());
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking backward");
            }
        }
    }
}

TEST_F(SoundSourceProxyTest, skipAndRead) {
    for (auto kReadFrameCount : kBufferSizes) {
        const QStringList filePaths = getFilePaths();
        for (const auto& filePath : filePaths) {
            ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));
            qDebug() << "Skip and read test:" << filePath;

            const auto fileUrl = QUrl::fromLocalFile(filePath);
            const auto providerRegistrations =
                    SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
            for (const auto& providerRegistration : providerRegistrations) {
                mixxx::AudioSourcePointer pContReadSource = openAudioSource(
                        filePath,
                        providerRegistration.getProvider());
                // Obtaining an AudioSource may fail for unsupported file formats,
                // even if the corresponding file extension is supported, e.g.
                // AAC vs. ALAC in .m4a files
                if (!pContReadSource) {
                    // skip test file
                    continue;
                }
                SINT contFrameIndex = pContReadSource->frameIndexMin();

                mixxx::AudioSourcePointer pSkipReadSource = openAudioSource(
                        filePath,
                        providerRegistration.getProvider());
                ASSERT_FALSE(!pSkipReadSource);
                ASSERT_EQ(
                        pContReadSource->getSignalInfo().getChannelCount(),
                        pSkipReadSource->getSignalInfo().getChannelCount());
                ASSERT_EQ(pContReadSource->frameIndexRange(), pSkipReadSource->frameIndexRange());
                SINT skipFrameIndex = pSkipReadSource->frameIndexMin();

                mixxx::SampleBuffer contReadData(
                        pContReadSource->getSignalInfo().frames2samples(kReadFrameCount));
                mixxx::SampleBuffer skipReadData(
                        pSkipReadSource->getSignalInfo().frames2samples(kReadFrameCount));

                SINT minFrameIndex = pContReadSource->frameIndexMin();
                SINT skipCount = 1;
                while (pContReadSource->frameIndexRange().containsIndex(
                        minFrameIndex += skipCount)) {
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
                    ASSERT_TRUE(contSampleFrames.frameIndexRange()
                                        .isSubrangeOf(readFrameIndexRange));
                    ASSERT_EQ(contSampleFrames.frameIndexRange().start(),
                            readFrameIndexRange.start());
                    contFrameIndex += contSampleFrames.frameLength();

                    const SINT sampleCount =
                            pContReadSource->getSignalInfo().frames2samples(
                                    contSampleFrames.frameLength());

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
                    ASSERT_EQ(contSampleFrames.frameIndexRange(),
                            skippedSampleFrames.frameIndexRange());
                    expectDecodedSamplesEqual(
                            sampleCount,
                            &contReadData[0],
                            &skipReadData[0],
                            "Decoding mismatch after skipping");

                    minFrameIndex = contFrameIndex;
                }
            }
        }
    }
}

TEST_F(SoundSourceProxyTest, seekBoundaries) {
    const SINT kReadFrameCount = 1000;
    const QStringList filePaths = getFilePaths();
    for (const auto& filePath : filePaths) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));
        qDebug() << "Seek boundaries test:" << filePath;

        const auto fileUrl = QUrl::fromLocalFile(filePath);
        const auto providerRegistrations =
                SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
        for (const auto& providerRegistration : providerRegistrations) {
            mixxx::AudioSourcePointer pSeekReadSource = openAudioSource(
                    filePath,
                    providerRegistration.getProvider());
            // Obtaining an AudioSource may fail for unsupported file formats,
            // even if the corresponding file extension is supported, e.g.
            // AAC vs. ALAC in .m4a files
            if (!pSeekReadSource) {
                // skip test file
                continue;
            }
            mixxx::SampleBuffer seekReadData(
                    pSeekReadSource->getSignalInfo().frames2samples(kReadFrameCount));

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
                    pSeekReadSource->frameIndexMax() - 4 * kReadFrameCount);
            seekFrameIndices.push_back(
                    pSeekReadSource->frameIndexMin() + pSeekReadSource->frameLength() / 2);
            // ...before the middle and then near the end of the stream...
            seekFrameIndices.push_back(pSeekReadSource->frameIndexMin() +
                    pSeekReadSource->frameLength() / 2 - 4 * kReadFrameCount);
            seekFrameIndices.push_back(
                    pSeekReadSource->frameIndexMax() - 4 * kReadFrameCount);
            // ...to the moddle of the stream and then skipping kReadFrameCount samples.
            seekFrameIndices.push_back(
                    pSeekReadSource->frameIndexMin() + pSeekReadSource->frameLength() / 2);
            seekFrameIndices.push_back(pSeekReadSource->frameIndexMin() +
                    pSeekReadSource->frameLength() / 2 + 2 * kReadFrameCount);

            // Read and verify results
            for (SINT seekFrameIndex : seekFrameIndices) {
                const auto readFrameIndexRange =
                        mixxx::IndexRange::forward(seekFrameIndex, kReadFrameCount);
                qDebug() << "Reading and verifying" << readFrameIndexRange;

                const auto expectedFrameIndexRange = intersect(
                        readFrameIndexRange,
                        pSeekReadSource->frameIndexRange());

                mixxx::AudioSourcePointer pContReadSource = openAudioSource(
                        filePath,
                        providerRegistration.getProvider());
                ASSERT_FALSE(!pContReadSource);
                ASSERT_EQ(
                        pSeekReadSource->getSignalInfo().getChannelCount(),
                        pContReadSource->getSignalInfo().getChannelCount());
                ASSERT_EQ(pSeekReadSource->frameIndexRange(), pContReadSource->frameIndexRange());
                const auto skipFrameIndexRange =
                        skipSampleFrames(pContReadSource,
                                mixxx::IndexRange::between(
                                        pContReadSource->frameIndexMin(),
                                        seekFrameIndex));
                ASSERT_TRUE(skipFrameIndexRange.empty() ||
                        (skipFrameIndexRange.end() == seekFrameIndex));
                mixxx::SampleBuffer contReadData(
                        pContReadSource->getSignalInfo().frames2samples(kReadFrameCount));
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
                        pSeekReadSource->getSignalInfo().frames2samples(
                                seekSampleFrames.frameLength());
                expectDecodedSamplesEqual(
                        sampleCount,
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking");
            }
        }
    }
}

TEST_F(SoundSourceProxyTest, readBeyondEnd) {
    const SINT kReadFrameCount = 1000;
    const QStringList filePaths = getFilePaths();
    for (const auto& filePath : filePaths) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));
        qDebug() << "read beyond end test:" << filePath;

        const auto fileUrl = QUrl::fromLocalFile(filePath);
        const auto providerRegistrations =
                SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
        for (const auto& providerRegistration : providerRegistrations) {
            mixxx::AudioSourcePointer pAudioSource = openAudioSource(
                    filePath,
                    providerRegistration.getProvider());
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

            mixxx::SampleBuffer readBuffer(
                    pAudioSource->getSignalInfo().frames2samples(kReadFrameCount));

            // Read beyond the end, starting within the valid range
            EXPECT_EQ(mixxx::IndexRange::forward(seekIndex, remainingFrames),
                    pAudioSource
                            ->readSampleFrames(mixxx::WritableSampleFrames(
                                    mixxx::IndexRange::forward(
                                            seekIndex, kReadFrameCount),
                                    mixxx::SampleBuffer::WritableSlice(
                                            readBuffer)))
                            .frameIndexRange());

            // Read beyond the end, starting at the upper boundary of the valid range
            EXPECT_EQ(mixxx::IndexRange::forward(pAudioSource->frameIndexMax(), 0),
                    pAudioSource
                            ->readSampleFrames(mixxx::WritableSampleFrames(
                                    mixxx::IndexRange::forward(
                                            pAudioSource->frameIndexMax(), kReadFrameCount),
                                    mixxx::SampleBuffer::WritableSlice(
                                            readBuffer)))
                            .frameIndexRange());

            // Read beyond the end, starting beyond the upper boundary of the valid range
            EXPECT_EQ(mixxx::IndexRange::forward(pAudioSource->frameIndexMax() + 1, 0),
                    pAudioSource
                            ->readSampleFrames(mixxx::WritableSampleFrames(
                                    mixxx::IndexRange::forward(
                                            pAudioSource->frameIndexMax() + 1, kReadFrameCount),
                                    mixxx::SampleBuffer::WritableSlice(
                                            readBuffer)))
                            .frameIndexRange());
        }
    }
}

TEST_F(SoundSourceProxyTest, regressionTestCachingReaderChunkJumpForward) {
    // NOTE(uklotzde, 2017-12-10): Potential regression test for an infinite
    // seek/read loop in SoundSourceMediaFoundation. Unfortunately this
    // test doesn't fail even prior to fixing the reported bug.
    // https://github.com/mixxxdj/mixxx/pull/1317#issuecomment-349674161
    const QStringList filePaths = getFilePaths();
    for (auto kReadFrameCount : kBufferSizes) {
        for (const auto& filePath : filePaths) {
            ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

            const auto fileUrl = QUrl::fromLocalFile(filePath);
            const auto providerRegistrations =
                    SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
            for (const auto& providerRegistration : providerRegistrations) {
                mixxx::AudioSourcePointer pAudioSource = openAudioSource(
                        filePath,
                        providerRegistration.getProvider());
                // Obtaining an AudioSource may fail for unsupported file formats,
                // even if the corresponding file extension is supported, e.g.
                // AAC vs. ALAC in .m4a files
                if (!pAudioSource) {
                    // skip test file
                    continue;
                }

                mixxx::SampleBuffer readBuffer(
                        pAudioSource->getSignalInfo().frames2samples(kReadFrameCount));

                // Read chunk from beginning
                auto firstChunkRange = mixxx::IndexRange::forward(
                        pAudioSource->frameIndexMin(), kReadFrameCount);
                EXPECT_EQ(
                        firstChunkRange,
                        pAudioSource->readSampleFrames(
                                            mixxx::WritableSampleFrames(
                                                    firstChunkRange,
                                                    mixxx::SampleBuffer::WritableSlice(readBuffer)))
                                .frameIndexRange());

                // Read chunk from near the end, rounded to chunk boundary
                auto secondChunkRange = mixxx::IndexRange::forward(
                        ((pAudioSource->frameIndexMax() - 2 * kReadFrameCount) /
                                kReadFrameCount) *
                                kReadFrameCount,
                        kReadFrameCount);
                EXPECT_EQ(
                        secondChunkRange,
                        pAudioSource->readSampleFrames(
                                            mixxx::WritableSampleFrames(
                                                    secondChunkRange,
                                                    mixxx::SampleBuffer::WritableSlice(readBuffer)))
                                .frameIndexRange());
            }
        }
    }
}
