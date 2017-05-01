#include <gmock/gmock.h>

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

    static mixxx::AudioSourcePointer openAudioSource(const QString& filePath) {
        TrackPointer pTrack(Track::newTemporary(filePath));
        return SoundSourceProxy(pTrack).openAudioSource();
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
        EXPECT_LT(0, pAudioSource->getChannelCount());
        EXPECT_LT(0, pAudioSource->getSamplingRate());
        EXPECT_LT(0, pAudioSource->getFrameCount());
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    TrackPointer pTrack(Track::newTemporary(
            kTestDir.absoluteFilePath("artist.mp3")));
    SoundSourceProxy proxy(pTrack);
    mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, proxy.parseTrackMetadata(&trackMetadata));
    EXPECT_EQ("Test Artist", trackMetadata.getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    TrackPointer pTrack(Track::newTemporary(
            kTestDir.absoluteFilePath("TOAL_TPE2.mp3")));
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

        for (SINT contFrameIndex = 0;
                pContReadSource->isValidFrameIndex(contFrameIndex);
                contFrameIndex += kReadFrameCount) {

            qDebug() << "Decoding from:" << contFrameIndex;

            // Read next chunk of frames for Cont source without seek
            const SINT contReadFrameCount =
                    pContReadSource->readSampleFrames(kReadFrameCount, &contReadData[0]);

            mixxx::AudioSourcePointer pSeekReadSource(openAudioSource(filePath));
            ASSERT_FALSE(!pSeekReadSource);
            ASSERT_EQ(pContReadSource->getChannelCount(), pSeekReadSource->getChannelCount());
            ASSERT_EQ(pContReadSource->getFrameCount(), pSeekReadSource->getFrameCount());

            // Seek source to next chunk and read it
            SINT seekFrameIndex =
                    pSeekReadSource->seekSampleFrame(contFrameIndex);
            ASSERT_EQ(contFrameIndex, seekFrameIndex);
            SINT seekReadFrameCount =
                    pSeekReadSource->readSampleFrames(kReadFrameCount, &seekReadData[0]);

            // Both buffers should be equal
            ASSERT_EQ(contReadFrameCount, seekReadFrameCount);
#ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        pContReadSource->frames2samples(contReadFrameCount),
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking forward");
            } else {
#endif // __OPUS__
                expectDecodedSamplesEqual(
                        pContReadSource->frames2samples(contReadFrameCount),
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking forward");
#ifdef __OPUS__
            }
#endif // __OPUS__

            // Seek backwards to beginning of chunk and read again
            seekFrameIndex =
                    pSeekReadSource->seekSampleFrame(contFrameIndex);
            ASSERT_EQ(contFrameIndex, seekFrameIndex);
            seekReadFrameCount =
                    pSeekReadSource->readSampleFrames(kReadFrameCount, &seekReadData[0]);

            // Both buffers should again be equal
            ASSERT_EQ(contReadFrameCount, seekReadFrameCount);
#ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        pContReadSource->frames2samples(contReadFrameCount),
                        &contReadData[0],
                        &seekReadData[0],
                        "Decoding mismatch after seeking backward");
            } else {
#endif // __OPUS__
                expectDecodedSamplesEqual(
                        pContReadSource->frames2samples(contReadFrameCount),
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

        mixxx::AudioSourcePointer pSkipReadSource(openAudioSource(filePath));
        ASSERT_FALSE(!pSkipReadSource);
        ASSERT_EQ(pContReadSource->getChannelCount(), pSkipReadSource->getChannelCount());
        ASSERT_EQ(pContReadSource->getFrameCount(), pSkipReadSource->getFrameCount());

        const SINT readSampleCount = pContReadSource->frames2samples(kReadFrameCount);
        SampleBuffer contReadData(readSampleCount);
        SampleBuffer skipReadData(readSampleCount);

        SINT frameIndex = mixxx::AudioSource::getMinFrameIndex();
        SINT contFrameIndex = mixxx::AudioSource::getMinFrameIndex();
        SINT skipFrameIndex = mixxx::AudioSource::getMinFrameIndex();
        SINT skipCount = 1;
        while (pContReadSource->isValidFrameIndex(frameIndex += skipCount)) {
            skipCount = frameIndex / 4 + 1;

            qDebug() << "Skipping to:" << frameIndex;

            // Read (and discard samples) until reaching the frame index
            // and read next chunk
            ASSERT_LE(contFrameIndex, frameIndex);
            while (contFrameIndex < frameIndex) {
                SINT readCount = std::min(frameIndex - contFrameIndex, kReadFrameCount);
                contFrameIndex += pContReadSource->readSampleFrames(readCount, &contReadData[0]);
            }
            ASSERT_EQ(contFrameIndex, frameIndex);
            const SINT contReadFrameCount =
                    pContReadSource->readSampleFrames(kReadFrameCount, &contReadData[0]);
            contFrameIndex += contReadFrameCount;

            // Skip until reaching the frame index and read next chunk
            ASSERT_LE(skipFrameIndex, frameIndex);
            skipFrameIndex +=
                    pSkipReadSource->skipSampleFrames(frameIndex - skipFrameIndex);
            ASSERT_EQ(skipFrameIndex, frameIndex);
            SINT skipReadFrameCount =
                    pSkipReadSource->readSampleFrames(kReadFrameCount, &skipReadData[0]);
            skipFrameIndex += skipReadFrameCount;

            // Both buffers should be equal
            ASSERT_EQ(contReadFrameCount, skipReadFrameCount);
#ifdef __OPUS__
            if (filePath.endsWith(".opus")) {
                expectDecodedSamplesEqualOpus(
                        pContReadSource->frames2samples(contReadFrameCount),
                        &contReadData[0],
                        &skipReadData[0],
                        "Decoding mismatch after skipping");
            } else {
#endif // __OPUS__
                expectDecodedSamplesEqual(
                        pContReadSource->frames2samples(contReadFrameCount),
                        &contReadData[0],
                        &skipReadData[0],
                        "Decoding mismatch after skipping");
#ifdef __OPUS__
            }
#endif // __OPUS__

            frameIndex = contFrameIndex;
        }
    }
}

TEST_F(SoundSourceProxyTest, seekBoundaries) {
    const SINT kReadFrameCount = 1000;

    for (const auto& filePath: getFilePaths()) {
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        qDebug() << "Seek boundaries test:" << filePath;

        mixxx::AudioSourcePointer pSeekReadSource(openAudioSource(filePath));
        // Obtaining an AudioSource may fail for unsupported file formats,
        // even if the corresponding file extension is supported, e.g.
        // AAC vs. ALAC in .m4a files
        if (!pSeekReadSource) {
            // skip test file
            continue;
        }

        // Seek to boundaries (alternating)
        EXPECT_EQ(pSeekReadSource->getMinFrameIndex(),
                pSeekReadSource->seekSampleFrame(pSeekReadSource->getMinFrameIndex()));
        EXPECT_EQ(pSeekReadSource->getMaxFrameIndex() - 1,
                pSeekReadSource->seekSampleFrame(pSeekReadSource->getMaxFrameIndex() - 1));
        EXPECT_EQ(pSeekReadSource->getMinFrameIndex() + 1,
                pSeekReadSource->seekSampleFrame(pSeekReadSource->getMinFrameIndex() + 1));
        EXPECT_EQ(pSeekReadSource->getMaxFrameIndex(),
                pSeekReadSource->seekSampleFrame(pSeekReadSource->getMaxFrameIndex()));

        // Seek to middle of the stream...
        const SINT frameOffset =
                (pSeekReadSource->getMaxFrameIndex() - pSeekReadSource->getMinFrameIndex()) / 2;
        const SINT frameIndex =
                mixxx::AudioSource::getMinFrameIndex() + frameOffset;
        EXPECT_EQ(frameIndex, pSeekReadSource->seekSampleFrame(frameIndex));

        // ...and verify read results
        mixxx::AudioSourcePointer pContReadSource(openAudioSource(filePath));
        ASSERT_FALSE(!pContReadSource);
        ASSERT_EQ(frameOffset, pContReadSource->skipSampleFrames(frameOffset));
        SampleBuffer contReadData(
                pContReadSource->frames2samples(kReadFrameCount));
        ASSERT_EQ(kReadFrameCount,
                pContReadSource->readSampleFrames(kReadFrameCount, &contReadData[0]));
        SampleBuffer seekReadData(
                pSeekReadSource->frames2samples(kReadFrameCount));
        ASSERT_EQ(kReadFrameCount,
                pSeekReadSource->readSampleFrames(kReadFrameCount, &seekReadData[0]));
#ifdef __OPUS__
        if (filePath.endsWith(".opus")) {
            expectDecodedSamplesEqualOpus(
                    pContReadSource->frames2samples(kReadFrameCount),
                    &contReadData[0],
                    &seekReadData[0],
                    "Decoding mismatch after seeking");
        } else {
#endif // __OPUS__
            expectDecodedSamplesEqual(
                    pContReadSource->frames2samples(kReadFrameCount),
                    &contReadData[0],
                    &seekReadData[0],
                    "Decoding mismatch after seeking");
#ifdef __OPUS__
        }
#endif // __OPUS__
    }
}
