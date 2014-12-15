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
