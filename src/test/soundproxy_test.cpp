#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>

#include "test/mixxxtest.h"
#include "soundsourceproxy.h"


class SoundSourceProxyTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pProxy = NULL;
    }

    virtual void TearDown() {
        delete m_pProxy;
    }

    Mixxx::SoundSource* loadProxy(const QString& track) {
        SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
            QDir(track), true);
        if (m_pProxy != NULL) {
            delete m_pProxy;
        }
        m_pProxy = new SoundSourceProxy(track, securityToken);
        Mixxx::SoundSource* pProxiedSoundSource = m_pProxy->getProxiedSoundSource();
        EXPECT_TRUE(pProxiedSoundSource != NULL && m_pProxy->parseHeader() == OK);
        return pProxiedSoundSource;
    }

    SoundSourceProxy* m_pProxy;
};


TEST_F(SoundSourceProxyTest, readArtist) {
    Mixxx::SoundSource* p = loadProxy(
        QDir::currentPath().append("/src/test/id3-test-data/artist.mp3"));
    EXPECT_EQ("Test Artist", p->getArtist());
}

TEST_F(SoundSourceProxyTest, TOAL_TPE2) {
    Mixxx::SoundSource* p = loadProxy(
        QDir::currentPath().append("/src/test/id3-test-data/TOAL_TPE2.mp3"));
    EXPECT_EQ("TITLE2", p->getArtist());
    EXPECT_EQ("ARTIST", p->getAlbum());
    EXPECT_EQ("TITLE", p->getAlbumArtist());
}
