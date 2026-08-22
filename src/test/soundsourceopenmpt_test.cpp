#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QtDebug>

#include "sources/audiosourcestereoproxy.h"
#include "sources/soundsourceopenmpt.h"
#include "sources/soundsourceproxy.h"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "util/samplebuffer.h"

namespace mixxx {

namespace {

const SINT kBufferSizes[] = {
        256,
        512,
        768,
        1024,
        1536,
        2048,
        3072,
        4096,
};

const SINT kMaxReadFrameCount =
        kBufferSizes[sizeof(kBufferSizes) / sizeof(kBufferSizes[0]) - 1];

const CSAMPLE kMaxDecodingError = 0.01f;

// Returns the path to the bundled test module file.
QString getTestModFilePath() {
    return QDir::current().filePath(
            QStringLiteral("src/test/id3-test-data/test.mod"));
}

} // anonymous namespace

// Tests SoundSourceOpenMPT using the same patterns as SoundSourceProxyTest:
// iterating over all provider registrations, testing multiple buffer sizes,
// and verifying seek consistency by comparing continuous-read and seek-read
// sample data.
class SoundSourceOpenMPTTest : public MixxxTest {
  protected:
    void SetUp() override {
        MixxxTest::SetUp();
        if (!SoundSourceProxy::isFileSuffixSupported("wav")) {
            SoundSourceProxy::registerProviders();
        }
    }

    static mixxx::AudioSourcePointer openAudioSource(
            const QString& filePath,
            const mixxx::SoundSourceProviderPointer& pProvider = nullptr) {
        auto pTrack = Track::newTemporary(filePath);
        SoundSourceProxy proxy(pTrack, pProvider);
        // Request stereo like SoundSourceProxyTest does
        mixxx::AudioSource::OpenParams openParams;
        openParams.setChannelCount(mixxx::audio::ChannelCount::stereo());
        auto pAudioSource = proxy.openAudioSource(openParams);
        if (pAudioSource) {
            if (pAudioSource->getSignalInfo().getChannelCount() !=
                    mixxx::audio::ChannelCount::stereo()) {
                pAudioSource = mixxx::AudioSourceStereoProxy::create(
                        pAudioSource,
                        kMaxReadFrameCount);
            }
        }
        return pAudioSource;
    }

    static void expectDecodedSamplesEqual(
            SINT size,
            const CSAMPLE* expected,
            const CSAMPLE* actual,
            const char* errorMessage) {
        for (SINT i = 0; i < size; ++i) {
            EXPECT_NEAR(expected[i], actual[i], kMaxDecodingError)
                    << "i=" << i << " " << errorMessage;
        }
    }

    mixxx::SampleBuffer m_skipSampleBuffer{kMaxReadFrameCount};
};

TEST_F(SoundSourceOpenMPTTest, open) {
    const QString filePath = getTestModFilePath();
    ASSERT_TRUE(QFile::exists(filePath));
    ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

    const auto fileUrl = QUrl::fromLocalFile(filePath);
    const auto providerRegistrations =
            SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);
    ASSERT_FALSE(providerRegistrations.isEmpty());

    for (const auto& providerRegistration : providerRegistrations) {
        auto pAudioSource = openAudioSource(
                filePath,
                providerRegistration.getProvider());
        ASSERT_NE(nullptr, pAudioSource);
        EXPECT_EQ(pAudioSource->getSignalInfo().getChannelCount(),
                  mixxx::audio::ChannelCount::stereo());
        EXPECT_EQ(pAudioSource->getSignalInfo().getSampleRate(),
                  SoundSourceOpenMPT::kSampleRate);
        EXPECT_FALSE(pAudioSource->frameIndexRange().empty());
    }
}

TEST_F(SoundSourceOpenMPTTest, openEmptyFile) {
    QTemporaryFile tmpFile("emptyXXXXXX.mod");
    ASSERT_FALSE(QFile::exists(tmpFile.fileName()));
    ASSERT_TRUE(tmpFile.open());
    const auto tmpFileName = tmpFile.fileName();
    ASSERT_TRUE(!tmpFileName.isEmpty());
    tmpFile.close();
    ASSERT_TRUE(QFile::exists(tmpFileName));

    ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(tmpFileName));
    auto pTrack = Track::newTemporary(tmpFileName);
    SoundSourceProxy proxy(pTrack);
    auto pAudioSource = proxy.openAudioSource();
    EXPECT_TRUE(!pAudioSource);
}

TEST_F(SoundSourceOpenMPTTest, openMissingFile) {
    const QString filePath = QStringLiteral("/nonexistent/path/test.mod");
    auto pTrack = Track::newTemporary(filePath);
    SoundSourceProxy proxy(pTrack);
    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(mixxx::audio::ChannelCount::stereo());
    auto pAudioSource = proxy.openAudioSource(openParams);
    EXPECT_EQ(nullptr, pAudioSource);
}

TEST_F(SoundSourceOpenMPTTest, seekForwardBackward) {
    constexpr SINT kReadFrameCount = 1000;

    const QString filePath = getTestModFilePath();
    ASSERT_TRUE(QFile::exists(filePath));
    ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

    const auto fileUrl = QUrl::fromLocalFile(filePath);
    const auto providerRegistrations =
            SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);

    for (const auto& providerRegistration : providerRegistrations) {
        auto pContReadSource = openAudioSource(
                filePath,
                providerRegistration.getProvider());
        ASSERT_NE(nullptr, pContReadSource);

        mixxx::SampleBuffer contReadData(
                pContReadSource->getSignalInfo().frames2samples(kReadFrameCount));
        mixxx::SampleBuffer seekReadData(
                pContReadSource->getSignalInfo().frames2samples(kReadFrameCount));

        SINT contFrameIndex = pContReadSource->frameIndexMin();
        while (pContReadSource->frameIndexRange().containsIndex(contFrameIndex)) {
            const auto readFrameIndexRange =
                    mixxx::IndexRange::forward(contFrameIndex, kReadFrameCount);

            // Read next chunk without seeking
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
                    pContReadSource->getSignalInfo().frames2samples(
                            contSampleFrames.frameLength());

            // Open a fresh source and seek to the same position
            auto pSeekReadSource = openAudioSource(
                    filePath,
                    providerRegistration.getProvider());
            ASSERT_NE(nullptr, pSeekReadSource);
            ASSERT_EQ(
                    pContReadSource->getSignalInfo().getChannelCount(),
                    pSeekReadSource->getSignalInfo().getChannelCount());
            ASSERT_EQ(pContReadSource->frameIndexRange(), pSeekReadSource->frameIndexRange());

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

            // Seek backwards and read again
            seekSampleFrames =
                    pSeekReadSource->readSampleFrames(
                            mixxx::WritableSampleFrames(
                                    readFrameIndexRange,
                                    mixxx::SampleBuffer::WritableSlice(seekReadData)));

            ASSERT_EQ(contSampleFrames.frameIndexRange(), seekSampleFrames.frameIndexRange());
            expectDecodedSamplesEqual(
                    sampleCount,
                    &contReadData[0],
                    &seekReadData[0],
                    "Decoding mismatch after seeking backward");
        }
    }
}

TEST_F(SoundSourceOpenMPTTest, readBeyondEnd) {
    constexpr SINT kReadFrameCount = 1000;

    const QString filePath = getTestModFilePath();
    ASSERT_TRUE(QFile::exists(filePath));
    ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

    const auto fileUrl = QUrl::fromLocalFile(filePath);
    const auto providerRegistrations =
            SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);

    for (const auto& providerRegistration : providerRegistrations) {
        auto pAudioSource = openAudioSource(
                filePath,
                providerRegistration.getProvider());
        ASSERT_NE(nullptr, pAudioSource);

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
    }
}

TEST_F(SoundSourceOpenMPTTest, multipleBufferSizes) {
    const QString filePath = getTestModFilePath();
    ASSERT_TRUE(QFile::exists(filePath));

    const auto fileUrl = QUrl::fromLocalFile(filePath);
    const auto providerRegistrations =
            SoundSourceProxy::allProviderRegistrationsForUrl(fileUrl);

    for (const auto& providerRegistration : providerRegistrations) {
        for (auto kReadFrameCount : kBufferSizes) {
            auto pAudioSource = openAudioSource(
                    filePath,
                    providerRegistration.getProvider());
            ASSERT_NE(nullptr, pAudioSource);

            mixxx::SampleBuffer readBuffer(
                    pAudioSource->getSignalInfo().frames2samples(kReadFrameCount));

            const auto readRange = pAudioSource->readSampleFrames(
                    mixxx::WritableSampleFrames(
                            mixxx::IndexRange::forward(0, kReadFrameCount),
                            mixxx::SampleBuffer::WritableSlice(readBuffer)));

            EXPECT_FALSE(readRange.frameIndexRange().empty());
            EXPECT_EQ(readRange.frameIndexRange().start(), 0);
        }
    }
}

} // namespace mixxx
