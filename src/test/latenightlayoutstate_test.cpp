#include <gtest/gtest.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QUrl>
#include <QVariant>

#include "test/mixxxtest.h"

namespace {

class LateNightLayoutStateTest : public MixxxTest {
  protected:
    std::unique_ptr<QObject> createState(bool maximizeLibrary,
            bool mixerVisible,
            int savedDeckSize,
            bool showCompactVuMetersSetting = true,
            bool showMaximizedDecks = true) {
        QQmlComponent component(
                &m_engine,
                QUrl::fromLocalFile(QStringLiteral(
                        RESOURCE_FOLDER "/skins/LateNightQML/LayoutState.qml")));
        QVariantMap initialProperties{
                {QStringLiteral("maximizeLibrary"), maximizeLibrary},
                {QStringLiteral("mixerVisible"), mixerVisible},
                {QStringLiteral("savedDeckSize"), savedDeckSize},
                {QStringLiteral("show4decks"), false},
                {QStringLiteral("showCompactVuMetersSetting"), showCompactVuMetersSetting},
                {QStringLiteral("showMaximizedDecks"), showMaximizedDecks},
        };
        QObject* object = component.createWithInitialProperties(initialProperties);
        EXPECT_FALSE(component.isError()) << qPrintable(component.errorString());
        EXPECT_NE(nullptr, object);
        return std::unique_ptr<QObject>(object);
    }

    QQmlEngine m_engine;
};

TEST_F(LateNightLayoutStateTest, MixerForcesFullWithoutChangingSavedSize) {
    auto state = createState(false, true, 0);
    ASSERT_TRUE(state);
    EXPECT_EQ(2, state->property("effectiveDeckSize").toInt());
    EXPECT_EQ(0, state->property("normalizedSavedDeckSize").toInt());

    state->setProperty("mixerVisible", false);
    EXPECT_EQ(0, state->property("effectiveDeckSize").toInt());
}

TEST_F(LateNightLayoutStateTest, BigLibraryTemporarilyUsesMiniAndRestoresSize) {
    auto state = createState(false, false, 1);
    ASSERT_TRUE(state);
    EXPECT_EQ(1, state->property("effectiveDeckSize").toInt());
    EXPECT_TRUE(state->property("showDeckArea").toBool());

    state->setProperty("maximizeLibrary", true);
    EXPECT_EQ(0, state->property("effectiveDeckSize").toInt());
    EXPECT_TRUE(state->property("showDeckArea").toBool());

    state->setProperty("maximizeLibrary", false);
    EXPECT_EQ(1, state->property("effectiveDeckSize").toInt());
}

TEST_F(LateNightLayoutStateTest, BigLibraryCanHideDecksAndCompactMeters) {
    auto state = createState(true, false, 1, true, false);
    ASSERT_TRUE(state);
    EXPECT_EQ(0, state->property("effectiveDeckSize").toInt());
    EXPECT_FALSE(state->property("showDeckArea").toBool());
    EXPECT_FALSE(state->property("showCompactVuMeters").toBool());
}

TEST_F(LateNightLayoutStateTest, SavedSizeIsClampedToSupportedRange) {
    auto state = createState(false, false, 99);
    ASSERT_TRUE(state);
    EXPECT_EQ(2, state->property("normalizedSavedDeckSize").toInt());
    state->setProperty("savedDeckSize", -10);
    EXPECT_EQ(0, state->property("normalizedSavedDeckSize").toInt());
}

TEST_F(LateNightLayoutStateTest, PlayerDropAreaParsesLibraryUriPayload) {
    QQmlComponent component(&m_engine);
    component.setData(R"(
import QtQuick 2.12
import Mixxx 1.0 as Mixxx

Item {
    Mixxx.PlayerDropArea {
        id: dropArea
        group: "[Channel1]"
        player: null
    }

    function parseUri() {
        return dropArea.firstDroppedUrl({
            formats: ["text/uri-list"],
            getDataAsString: function(format) { return "file:///tmp/Late%20Night.wav\n"; },
            hasUrls: false,
            urls: []
        });
    }
}
)",
            QUrl::fromLocalFile(QStringLiteral(
                    RESOURCE_FOLDER "/skins/LateNightQML/playerdroparea_test.qml")));
    std::unique_ptr<QObject> object(component.create());
    ASSERT_FALSE(component.isError()) << qPrintable(component.errorString());
    ASSERT_TRUE(object);

    QVariant parsedUrl;
    ASSERT_TRUE(QMetaObject::invokeMethod(
            object.get(), "parseUri", Q_RETURN_ARG(QVariant, parsedUrl)));
    EXPECT_EQ(QStringLiteral("/tmp/Late Night.wav"), parsedUrl.toUrl().toLocalFile());
}

} // namespace
