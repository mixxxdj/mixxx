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
            QDir::currentPath() + "/src/test/id3-test-data/cover-test");

    QStringList extensions;
    extensions << ".aiff" << ".flac" << "-png.mp3" << ".ogg" << ".wav";

#ifdef __OPUS__
    extensions << ".opus";
#endif

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

TEST_F(SoundSourceProxyTest, seekTheSame) {
    const unsigned int kSeekFrameIndex = 10000;
    const unsigned int kTestFrameCount = 10;

    const QString kFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "mp3" << "ogg" << "wav";

    foreach (const QString& extension, extensions) {
        QString filePath = kFilePath + extension;

        Mixxx::AudioSourcePointer pAudioSource1(
            openAudioSource(filePath));
        EXPECT_FALSE(pAudioSource1.isNull());
        const unsigned int sampleCount1 = pAudioSource1->frames2samples(kTestFrameCount);
        CSAMPLE *pData1 = new CSAMPLE[sampleCount1];
        pAudioSource1->seekSampleFrame(kSeekFrameIndex);
        unsigned int readCount1 = pAudioSource1->readSampleFrames(kTestFrameCount, pData1);
        EXPECT_EQ(readCount1, kTestFrameCount);

        Mixxx::AudioSourcePointer pAudioSource2(
            openAudioSource(filePath));
        EXPECT_FALSE(pAudioSource2.isNull());
        const unsigned int sampleCount2 = pAudioSource2->frames2samples(kTestFrameCount);
        CSAMPLE *pData2 = new CSAMPLE[sampleCount2];
        pAudioSource2->seekSampleFrame(kSeekFrameIndex);
        unsigned int readCount2 = pAudioSource2->readSampleFrames(kTestFrameCount, pData2);
        EXPECT_EQ(readCount2, kTestFrameCount);

        EXPECT_EQ(readCount1, readCount2);
        for (unsigned int i = 0; i < readCount1; i++) {
            if (pData1[i] != pData2[i]) {
                qDebug() << filePath << "Test Sample"  << i;
            }
            EXPECT_EQ(pData1[i], pData2[i]);
            //qDebug() << pData1[i];
        }

        delete[] pData1;
        delete[] pData2;
    }
}


TEST_F(SoundSourceProxyTest, readBeforeSeek) {
    const unsigned int kSeekFrameIndex = 10000;
    const unsigned int kTestFrameCount = 10;

    const QString kFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "mp3" << "ogg" << "wav";

    foreach (const QString& extension, extensions) {
        QString filePath = kFilePath + extension;

        Mixxx::AudioSourcePointer pAudioSource1(
            openAudioSource(filePath));
        EXPECT_FALSE(pAudioSource1.isNull());
        const unsigned int sampleCount1 = pAudioSource1->frames2samples(kTestFrameCount);
        CSAMPLE *pData1 = new CSAMPLE[sampleCount1];
        unsigned int readCount1 = pAudioSource1->readSampleFrames(kTestFrameCount, pData1);
        EXPECT_EQ(readCount1, kTestFrameCount);
        pAudioSource1->seekSampleFrame(kSeekFrameIndex);
        readCount1 = pAudioSource1->readSampleFrames(kTestFrameCount, pData1);
        EXPECT_EQ(readCount1, kTestFrameCount);

        Mixxx::AudioSourcePointer pAudioSource2(
            openAudioSource(filePath));
        EXPECT_FALSE(pAudioSource2.isNull());
        const unsigned int sampleCount2 = pAudioSource2->frames2samples(kTestFrameCount);
        CSAMPLE *pData2 = new CSAMPLE[sampleCount2];
        pAudioSource2->seekSampleFrame(kSeekFrameIndex);
        unsigned int readCount2 = pAudioSource2->readSampleFrames(kTestFrameCount, pData2);
        EXPECT_EQ(readCount2, kTestFrameCount);

        EXPECT_EQ(readCount1, readCount2);
        for (unsigned int i = 0; i < readCount1; i++) {
            if (pData1[i] != pData2[i]) {
                qDebug() << filePath << "Test Sample"  << i;
            }
            EXPECT_EQ(pData1[i], pData2[i]);
            //qDebug() << pData1[i];
        }

        delete[] pData1;
        delete[] pData2;
    }
}

TEST_F(SoundSourceProxyTest, seekForward) {
    const unsigned int kSeekFrameIndex = 10000;
    const unsigned int kTestFrameCount = 10;

    const QString kFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "mp3" << "ogg" << "wav";

    foreach (const QString& extension, extensions) {
        QString filePath = kFilePath + extension;

        Mixxx::AudioSourcePointer pAudioSource1(
            openAudioSource(filePath));
        EXPECT_FALSE(pAudioSource1.isNull());
        const unsigned int sampleCount1 = pAudioSource1->frames2samples(kTestFrameCount);
        CSAMPLE *pData1 = new CSAMPLE[sampleCount1];
        unsigned int readCount1;
        for (unsigned int i = 0; i <= kSeekFrameIndex / kTestFrameCount; ++i) {
            readCount1 = pAudioSource1->readSampleFrames(kTestFrameCount, pData1);
            EXPECT_EQ(readCount1, kTestFrameCount);
        }

        Mixxx::AudioSourcePointer pAudioSource2(
            openAudioSource(filePath));
        EXPECT_FALSE(pAudioSource2.isNull());
        const unsigned int sampleCount2 = pAudioSource2->frames2samples(kTestFrameCount);
        CSAMPLE *pData2 = new CSAMPLE[sampleCount2];
        pAudioSource2->seekSampleFrame(kSeekFrameIndex);
        unsigned int readCount2 = pAudioSource2->readSampleFrames(kTestFrameCount, pData2);
        EXPECT_EQ(readCount2, kTestFrameCount);

        EXPECT_EQ(readCount1, readCount2);
        for (unsigned int i = 0; i < readCount1; i++) {
            if (pData1[i] != pData2[i]) {
                qDebug() << filePath << "Test Sample"  << i;
            }
            EXPECT_EQ(pData1[i], pData2[i]);
            //qDebug() << pData1[i];
        }

        delete[] pData1;
        delete[] pData2;
    }

}



