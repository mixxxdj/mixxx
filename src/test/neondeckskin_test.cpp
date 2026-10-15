#include <gtest/gtest.h>

#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QString>

#include "skin/qml/qmlskin.h"

namespace {

// The NeonDeck skin ships with Mixxx. Guard its structure so a broken skin is
// caught by the test suite instead of by the user at start-up.
TEST(NeonDeckSkinTest, IsAValidQmlSkin) {
    const auto pSkin = mixxx::skin::qml::QmlSkin::fromDirectory(
            QDir(QStringLiteral(RESOURCE_FOLDER "/skins/NeonDeck")));
    ASSERT_TRUE(pSkin);
    EXPECT_TRUE(pSkin->isValid());
    EXPECT_EQ(mixxx::skin::SkinType::QML, pSkin->type());
    EXPECT_FALSE(pSkin->mainQmlFilePath().isEmpty());
}

TEST(NeonDeckSkinTest, AdvertisesItsOwnColorScheme) {
    const auto pSkin = mixxx::skin::qml::QmlSkin::fromDirectory(
            QDir(QStringLiteral(RESOURCE_FOLDER "/skins/NeonDeck")));
    ASSERT_TRUE(pSkin);
    EXPECT_TRUE(pSkin->colorschemes().contains(QStringLiteral("NeonDeck")))
            << "the NeonDeck skin does not advertise its own color scheme";
}

TEST(NeonDeckSkinTest, ColorSchemeIsDeclaredForTheTheme) {
    EXPECT_TRUE(QFileInfo::exists(QStringLiteral(
            RESOURCE_FOLDER "/skins/NeonDeck/LateNightTheme/ColorSchemes/NeonDeck.qml")));
}

} // namespace
