#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "test/mixxxtest.h"
#include "soundsourceproxy.h"


class SoundSourceProxyTest : public MixxxTest {
  protected:
    Mixxx::SoundSourcePointer loadProxy(const QString& track) {
        m_pProxy.reset(new SoundSourceProxy(track, SecurityTokenPointer()));
        return m_pProxy->getSoundSource();
    }

    QScopedPointer<SoundSourceProxy> m_pProxy;
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

        Mixxx::SoundSourcePointer pSoundSource(loadProxy(filePath));
        ASSERT_TRUE(!pSoundSource.isNull());
        EXPECT_EQ(OK, pSoundSource->open());
        EXPECT_LT(0, pSoundSource->getChannels());
        EXPECT_LT(0UL, pSoundSource->getSampleRate());
        EXPECT_LT(0UL, pSoundSource->length());
    }
}

TEST_F(SoundSourceProxyTest, readArtist) {
    Mixxx::SoundSourcePointer p(loadProxy(
        QDir::currentPath().append("/src/test/id3-test-data/artist.mp3")));
    ASSERT_TRUE(!p.isNull());
    EXPECT_EQ(OK, p->parseHeader());
    EXPECT_EQ("Test Artist", p->getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    Mixxx::SoundSourcePointer p(loadProxy(
        QDir::currentPath().append("/src/test/id3-test-data/TOAL_TPE2.mp3")));
    ASSERT_TRUE(!p.isNull());
    EXPECT_EQ(OK, p->parseHeader());
    EXPECT_EQ("TITLE2", p->getArtist());
    EXPECT_EQ("ARTIST", p->getAlbum());
    EXPECT_EQ("TITLE", p->getAlbumArtist());
}

TEST_F(SoundSourceProxyTest, seekTheSame) {
    const int kTestSampleCount = 10;
    const int kSeekSample = 10000;

    const QString kFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "mp3" << "ogg" << "wav";
    
    SAMPLE *pData1 = new SAMPLE[kTestSampleCount];
    SAMPLE *pData2 = new SAMPLE[kTestSampleCount];

    foreach (const QString& extension, extensions) {
        QString filePath = kFilePath + extension;

        Mixxx::SoundSourcePointer pSoundSource1(loadProxy(filePath));
        EXPECT_EQ(OK, pSoundSource1->open());
        pSoundSource1->seek(kSeekSample);
        unsigned int read1 = pSoundSource1->read(kTestSampleCount, pData1);
        EXPECT_EQ(read1, kTestSampleCount);

        Mixxx::SoundSourcePointer pSoundSource2(loadProxy(filePath));
        EXPECT_EQ(OK, pSoundSource2->open());
        pSoundSource2->seek(kSeekSample);
        unsigned int read2 = pSoundSource2->read(kTestSampleCount, pData2);
        EXPECT_EQ(read2, kTestSampleCount);

        for( int i = 0; i < kTestSampleCount; i++) {
            EXPECT_EQ(pData1[i], pData2[i]);
            if (pData1[i] != pData2[i]) {
                qDebug() << filePath << "Test Sample"  << i;
                break;
            }
            //qDebug() << pData1[i];
        }
    }
    delete[] pData1;
    delete[] pData2;
}


TEST_F(SoundSourceProxyTest, readBeforeSeek) {
    const int kTestSampleCount = 10;
    const int kSeekSample = 10000;

    const QString kFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "mp3" << "ogg" << "wav";

    SAMPLE *pData1 = new SAMPLE[kTestSampleCount];
    SAMPLE *pData2 = new SAMPLE[kTestSampleCount];

    foreach (const QString& extension, extensions) {
        QString filePath = kFilePath + extension;

        Mixxx::SoundSourcePointer pSoundSource1(loadProxy(filePath));
        EXPECT_EQ(OK, pSoundSource1->open());
        SAMPLE *pData1 = new SAMPLE[kTestSampleCount];
        unsigned int read1 = pSoundSource1->read(kTestSampleCount, pData1);
        EXPECT_EQ(read1, kTestSampleCount);
        pSoundSource1->seek(kSeekSample);
        read1 = pSoundSource1->read(kTestSampleCount, pData1);
        EXPECT_EQ(read1, kTestSampleCount);

        Mixxx::SoundSourcePointer pSoundSource2(loadProxy(filePath));
        EXPECT_EQ(OK, pSoundSource2->open());
        SAMPLE *pData2 = new SAMPLE[kTestSampleCount];
        pSoundSource2->seek(kSeekSample);
        unsigned int read2 = pSoundSource2->read(kTestSampleCount, pData2);
        EXPECT_EQ(read2, kTestSampleCount);

        for( int i = 0; i < kTestSampleCount; i++) {
            EXPECT_EQ(pData1[i], pData2[i]);
            if (pData1[i] != pData2[i]) {
                qDebug() << filePath << "Test Sample"  << i;
                break;
            }
            //qDebug() << pData1[i];
        }
    }
    delete[] pData1;
    delete[] pData2;
}

TEST_F(SoundSourceProxyTest, seekForward) {
    const unsigned int kSeekFrameIndex = 10000;
    const unsigned int kTestFrameCount = 100;

    EXPECT_EQ(0, int(kSeekFrameIndex % kTestFrameCount));

    const QString kFilePath(
            QDir::currentPath() + "/src/test/id3-test-data/cover-test.");

    QStringList extensions;
    extensions << "aiff" << "flac" << "wav"  << "ogg" << "mp3";

    SAMPLE *pData1 = new SAMPLE[kTestFrameCount];
    SAMPLE *pData2 = new SAMPLE[kTestFrameCount];

    for (unsigned int seekFrameIndex = 0; 1200000 > seekFrameIndex; seekFrameIndex += kSeekFrameIndex) {
        qDebug() << "seekFrameIndex =" << seekFrameIndex;
        foreach (const QString& extension, extensions) {
            QString filePath = kFilePath + extension;

            Mixxx::SoundSourcePointer pSoundSource1(loadProxy(filePath));
            EXPECT_EQ(OK, pSoundSource1->open());
            unsigned int frameIndex1 = 0;
            while (frameIndex1 < seekFrameIndex) {
                unsigned int readCount1 = pSoundSource1->read(kTestFrameCount, pData1);
                EXPECT_EQ(kTestFrameCount, readCount1);
                frameIndex1 += readCount1;
            }
            EXPECT_EQ(seekFrameIndex, frameIndex1);
            unsigned int readCount1 = pSoundSource1->read(kTestFrameCount, pData1);

            Mixxx::SoundSourcePointer pSoundSource2(loadProxy(filePath));
            EXPECT_EQ(OK, pSoundSource2->open());
            pSoundSource2->seek(seekFrameIndex);
            unsigned int read2 = pSoundSource2->read(kTestFrameCount, pData2);
            EXPECT_EQ(read2, kTestFrameCount);

            for( int i = 0; i < kTestFrameCount; i++) {
                ASSERT_EQ(pData1[i], pData2[i]);
                if (pData1[i] != pData2[i]) {
                    qDebug() << filePath << "Test Sample" << i;
                    break;
                }
            }
        }
    }
    delete[] pData1;
    delete[] pData2;
}
