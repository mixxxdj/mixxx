#include <gtest/gtest.h>

#include <QDir>
#include <QFileInfo>
#include <QString>

#include "skin/qml/qmlskin.h"
#include "skin/skinloader.h"
#include "test/mixxxtest.h"

namespace {

// PulseDeck is the skin a fresh installation starts with, so its structure is
// guarded here. A broken skin then fails the test suite instead of the user at
// start-up.
class PulseDeckSkinTest : public MixxxTest {
};

TEST_F(PulseDeckSkinTest, IsAValidQmlSkin) {
    const auto pSkin = mixxx::skin::qml::QmlSkin::fromDirectory(
            QDir(QStringLiteral(RESOURCE_FOLDER "/skins/PulseDeck")));
    ASSERT_TRUE(pSkin);
    EXPECT_TRUE(pSkin->isValid());
    EXPECT_EQ(mixxx::skin::SkinType::QML, pSkin->type());
    EXPECT_FALSE(pSkin->mainQmlFilePath().isEmpty());
}

TEST_F(PulseDeckSkinTest, IsTheDefaultSkin) {
    const mixxx::skin::SkinLoader skinLoader(config());
    EXPECT_EQ(QStringLiteral("PulseDeck"), skinLoader.getDefaultSkinName())
            << "a fresh install would start on a skin that is not the one the "
               "console was designed around";
}

TEST_F(PulseDeckSkinTest, DefaultSkinShipsWithTheResourceFiles) {
    const mixxx::skin::SkinLoader skinLoader(config());
    const QDir defaultSkinDir(
            QStringLiteral(RESOURCE_FOLDER "/skins/") + skinLoader.getDefaultSkinName());
    EXPECT_TRUE(defaultSkinDir.exists())
            << "the default skin name does not match any directory in res/skins";
}

TEST_F(PulseDeckSkinTest, AdvertisesTheNeonDeckColorScheme) {
    const auto pSkin = mixxx::skin::qml::QmlSkin::fromDirectory(
            QDir(QStringLiteral(RESOURCE_FOLDER "/skins/PulseDeck")));
    ASSERT_TRUE(pSkin);
    EXPECT_TRUE(pSkin->colorschemes().contains(QStringLiteral("NeonDeck")))
            << "the PulseDeck skin does not advertise its own color scheme";
}

// The theme falls back to the NeonDeck palette when no scheme is configured,
// which is what happens on a fresh install. Without this file the deck colours
// and the two tone waveforms would silently disappear.
TEST_F(PulseDeckSkinTest, ShipsThePaletteItFallsBackTo) {
    EXPECT_TRUE(QFileInfo::exists(QStringLiteral(
            RESOURCE_FOLDER "/skins/PulseDeck/LateNightTheme/ColorSchemes/NeonDeck.qml")));
}

} // namespace
