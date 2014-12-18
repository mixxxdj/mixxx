#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>

#include "test/mixxxtest.h"

#include "soundsourceproxy.h"
#include "metadata/trackmetadata.h"

class SoundSourceProxyTest : public MixxxTest {
  protected:
    Mixxx::SoundSourcePointer openSoundSource(const QString& fileName) {
        return SoundSourceProxy(fileName, SecurityTokenPointer()).getSoundSource();
    }
    Mixxx::AudioSourcePointer openAudioSource(const QString& fileName) {
        return SoundSourceProxy(fileName, SecurityTokenPointer()).openAudioSource();
    }
};

TEST_F(SoundSourceProxyTest, ProxyCanOpen) {
    // This test piggy-backs off of the cover-test files.
    const QString kCoverFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "mp3" << "ogg" << "wav";

    foreach (const QString& extension, extensions) {
        QString filePath = kCoverFilePath + extension;
        EXPECT_TRUE(SoundSourceProxy::isFilenameSupported(filePath));

        Mixxx::AudioSourcePointer pAudioSource(openAudioSource(filePath));
        ASSERT_TRUE(!pAudioSource.isNull());
        EXPECT_LT(0UL, pAudioSource->getChannelCount());
        EXPECT_LT(0UL, pAudioSource->getFrameRate());
        EXPECT_LT(0UL, pAudioSource->getFrameCount());
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    Mixxx::SoundSourcePointer pSoundSource(openSoundSource(
        QDir::currentPath().append("/src/test/id3-test-data/artist.mp3")));
    ASSERT_TRUE(!pSoundSource.isNull());
    Mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, pSoundSource->parseMetadata(&trackMetadata));
    EXPECT_EQ("Test Artist", trackMetadata.getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    Mixxx::SoundSourcePointer pSoundSource(openSoundSource(
        QDir::currentPath().append("/src/test/id3-test-data/TOAL_TPE2.mp3")));
    ASSERT_TRUE(!pSoundSource.isNull());
    Mixxx::TrackMetadata trackMetadata;
    EXPECT_EQ(OK, pSoundSource->parseMetadata(&trackMetadata));
    EXPECT_EQ("TITLE2", trackMetadata.getArtist());
    EXPECT_EQ("ARTIST", trackMetadata.getAlbum());
    EXPECT_EQ("TITLE", trackMetadata.getAlbumArtist());
}
