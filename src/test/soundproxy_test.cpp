#include "test/mixxxtest.h"

#include "soundsourceproxy.h"
#include "metadata/trackmetadata.h"
#include "samplebuffer.h"

#include <gmock/gmock.h>

#include <QtDebug>

class SoundSourceProxyTest: public MixxxTest {
  protected:
    static QStringList getFileExtensions() {
        QStringList availableExtensions;
        availableExtensions << "aiff" << "flac" << "m4a" << "mp3" << "ogg" << "opus" << "wav";
        QStringList supportedExtensions;
        foreach (QString const& fileExtension, availableExtensions) {
            if (SoundSourceProxy::isFileExtensionSupported(fileExtension)) {
                supportedExtensions << fileExtension;
            }
        }
        return supportedExtensions;
    }

    static Mixxx::AudioSourcePointer openAudioSource(const QString& fileName) {
        return SoundSourceProxy(fileName).openAudioSource();
    }
};

TEST_F(SoundSourceProxyTest, open) {
    // This test piggy-backs off of the cover-test files.
    const QString kFilePathPrefix(
        QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    foreach (const QString& fileExtension, getFileExtensions()) {
        const QString filePath(kFilePathPrefix + fileExtension);
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        Mixxx::AudioSourcePointer pAudioSource(openAudioSource(filePath));
        ASSERT_TRUE(!pAudioSource.isNull());
        EXPECT_LT(0, pAudioSource->getChannelCount());
        EXPECT_LT(0, pAudioSource->getFrameRate());
        EXPECT_LT(0, pAudioSource->getFrameCount());
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    SoundSourceProxy proxy(
        QDir::currentPath().append("/src/test/id3-test-data/artist.mp3"));
    Mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, proxy.parseTrackMetadataAndCoverArt(&trackMetadata, NULL));
    EXPECT_EQ("Test Artist", trackMetadata.getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    SoundSourceProxy proxy(
        QDir::currentPath().append("/src/test/id3-test-data/TOAL_TPE2.mp3"));
    Mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, proxy.parseTrackMetadataAndCoverArt(&trackMetadata, NULL));
    EXPECT_EQ("TITLE2", trackMetadata.getArtist());
    EXPECT_EQ("ARTIST", trackMetadata.getAlbum());
    EXPECT_EQ("TITLE", trackMetadata.getAlbumArtist());
}

TEST_F(SoundSourceProxyTest, seekForward) {
    const SINT kReadFrameCount = 10000;

    // According to API documentation of op_pcm_seek():
    // "...decoding after seeking may not return exactly the same
    // values as would be obtained by decoding the stream straight
    // through. However, such differences are expected to be smaller
    // than the loss introduced by Opus's lossy compression."
    // NOTE(uklotzde): The current version 0.6 of opusfile doesn't
    // seem to support sample accurate seeking. The differences
    // between the samples decoded with continuous reading and
    // those samples decoded after seeking are quite noticeable!
    const CSAMPLE kOpusSeekDecodingError = 0.2f;

    const QString kFilePathPrefix(
        QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    foreach (const QString& fileExtension, getFileExtensions()) {
        const QString filePath(kFilePathPrefix + fileExtension);
        ASSERT_TRUE(SoundSourceProxy::isFileNameSupported(filePath));

        qDebug() << "Seek forward test:" << filePath;

        Mixxx::AudioSourcePointer pContReadSource(openAudioSource(filePath));
        ASSERT_FALSE(pContReadSource.isNull());
        const SINT readSampleCount = pContReadSource->frames2samples(kReadFrameCount);
        SampleBuffer contReadData(readSampleCount);
        SampleBuffer seekReadData(readSampleCount);

        for (SINT contFrameIndex = 0;
                pContReadSource->isValidFrameIndex(contFrameIndex);
                contFrameIndex += kReadFrameCount) {

            const SINT contReadFrameCount =
                pContReadSource->readSampleFrames(kReadFrameCount, &contReadData[0]);

            Mixxx::AudioSourcePointer pSeekReadSource(openAudioSource(filePath));
            ASSERT_FALSE(pSeekReadSource.isNull());
            ASSERT_EQ(pContReadSource->getChannelCount(), pSeekReadSource->getChannelCount());
            ASSERT_EQ(pContReadSource->getFrameCount(), pSeekReadSource->getFrameCount());

            const SINT seekFrameIndex =
                pSeekReadSource->seekSampleFrame(contFrameIndex);
            ASSERT_EQ(contFrameIndex, seekFrameIndex);

            const SINT seekReadFrameCount =
                pSeekReadSource->readSampleFrames(kReadFrameCount, &seekReadData[0]);

            ASSERT_EQ(contReadFrameCount, seekReadFrameCount);
            const SINT readSampleCount =
                pContReadSource->frames2samples(contReadFrameCount);
            for (SINT readSampleOffset = 0;
                    readSampleOffset < readSampleCount;
                    ++readSampleOffset) {
                if ("opus" == fileExtension) {
                    EXPECT_NEAR(contReadData[readSampleOffset], seekReadData[readSampleOffset], kOpusSeekDecodingError)
                            << "Mismatch in " << filePath.toStdString()
                            << " at seek frame index " << seekFrameIndex
                            << " for read sample offset " << readSampleOffset;
                } else {
                    // NOTE(uklotzde): The comparison EXPECT_EQ might be
                    // replaced with EXPECT_FLOAT_EQ to guarantee almost
                    // accurate seeking. Currently EXPECT_EQ works for all
                    // tested file formats except Opus.
                    EXPECT_EQ(contReadData[readSampleOffset], seekReadData[readSampleOffset])
                            << "Mismatch in " << filePath.toStdString()
                            << " at seek frame index " << seekFrameIndex
                            << " for read sample offset " << readSampleOffset;
                }
            }
        }
    }

}



