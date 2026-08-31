#include <gtest/gtest.h>

#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QMetaProperty>
#include <QMetaType>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <memory>

#include "qml/qmlconfigproxy.h"
#include "test/mixxxtest.h"

namespace {

class ThemeQmlTest : public MixxxTest {
  protected:
    QObject* loadTheme() {
        mixxx::qml::QmlConfigProxy::registerUserSettings(config());
        m_engine.addImportPath(QStringLiteral(RESOURCE_FOLDER "/qml"));

        QQmlComponent component(&m_engine);
        component.setData(R"(
import QtQuick 2.12
import "Theme"

QtObject {
    readonly property var theme: Theme
}
)",
                QUrl::fromLocalFile(QStringLiteral(RESOURCE_FOLDER "/qml/themeqml_test.qml")));

        std::unique_ptr<QObject> pRoot(component.create());
        EXPECT_FALSE(component.isError()) << qPrintable(component.errorString());
        EXPECT_TRUE(pRoot) << qPrintable(component.errorString());
        if (!pRoot) {
            return nullptr;
        }

        QObject* pTheme = pRoot->property("theme").value<QObject*>();
        EXPECT_TRUE(pTheme);
        if (!pTheme) {
            return nullptr;
        }

        m_pThemeRoot = std::move(pRoot);
        return pTheme;
    }

    QObject* loadLateNightTheme(const QString& schemeName) {
        config()->set(ConfigKey(QStringLiteral("[Config]"), QStringLiteral("Scheme")), schemeName);
        mixxx::qml::QmlConfigProxy::registerUserSettings(config());
        m_engine.addImportPath(QStringLiteral(RESOURCE_FOLDER "/qml"));

        QQmlComponent component(&m_engine);
        component.setData(R"(
import QtQuick 2.12
import "LateNightTheme"

QtObject {
    readonly property var theme: LateNightTheme
}
)",
                QUrl::fromLocalFile(QStringLiteral(
                        RESOURCE_FOLDER "/skins/LateNightQML/latenightthemeqml_test.qml")));

        std::unique_ptr<QObject> pRoot(component.create());
        EXPECT_FALSE(component.isError()) << qPrintable(component.errorString());
        EXPECT_TRUE(pRoot) << qPrintable(component.errorString());
        if (!pRoot) {
            return nullptr;
        }

        QObject* pTheme = pRoot->property("theme").value<QObject*>();
        EXPECT_TRUE(pTheme);
        if (!pTheme) {
            return nullptr;
        }

        m_pThemeRoot = std::move(pRoot);
        return pTheme;
    }

  private:
    QQmlEngine m_engine;
    std::unique_ptr<QObject> m_pThemeRoot;
};

void assertLateNightAssetPropertiesReferenceExistingFiles(const QObject* pTheme) {
    const QMetaObject* pMetaObject = pTheme->metaObject();
    for (int i = pMetaObject->propertyOffset(); i < pMetaObject->propertyCount(); ++i) {
        const QMetaProperty property = pMetaObject->property(i);
        const QString propertyName = QString::fromLatin1(property.name());
        if (!propertyName.startsWith(QStringLiteral("asset"))) {
            continue;
        }

        SCOPED_TRACE(property.name());

        const QUrl assetUrl = property.read(pTheme).toUrl();
        ASSERT_TRUE(assetUrl.isLocalFile()) << qPrintable(assetUrl.toString());

        const QFileInfo assetFileInfo(assetUrl.toLocalFile());
        EXPECT_TRUE(assetFileInfo.exists())
                << qPrintable(assetFileInfo.absoluteFilePath());
    }
}

} // namespace

TEST_F(ThemeQmlTest, ColorPropertiesAreValid) {
    const QObject* pTheme = loadTheme();
    ASSERT_TRUE(pTheme);

    const QMetaObject* pMetaObject = pTheme->metaObject();
    for (int i = pMetaObject->propertyOffset(); i < pMetaObject->propertyCount(); ++i) {
        const QMetaProperty property = pMetaObject->property(i);
        if (property.userType() != QMetaType::QColor) {
            continue;
        }

        SCOPED_TRACE(property.name());
        const QVariant value = property.read(pTheme);
        ASSERT_TRUE(value.isValid());

        const QColor color = value.value<QColor>();
        EXPECT_TRUE(color.isValid());
    }
}

TEST_F(ThemeQmlTest, ImagePropertiesReferenceExistingSvgs) {
    const QObject* pTheme = loadTheme();
    ASSERT_TRUE(pTheme);

    const QDir qmlResourceDir(QStringLiteral(RESOURCE_FOLDER "/qml"));
    const QMetaObject* pMetaObject = pTheme->metaObject();
    for (int i = pMetaObject->propertyOffset(); i < pMetaObject->propertyCount(); ++i) {
        const QMetaProperty property = pMetaObject->property(i);
        const QString propertyName = QString::fromLatin1(property.name());
        if (!propertyName.startsWith(QStringLiteral("img"))) {
            continue;
        }

        SCOPED_TRACE(property.name());

        const QString imagePath = property.read(pTheme).toString();
        ASSERT_FALSE(imagePath.isEmpty());

        const QFileInfo imageFileInfo(qmlResourceDir.filePath(imagePath));
        EXPECT_QSTRING_EQ(QStringLiteral("svg"), imageFileInfo.suffix());
        EXPECT_TRUE(imageFileInfo.exists())
                << qPrintable(imageFileInfo.absoluteFilePath());
    }
}

TEST_F(ThemeQmlTest, LateNightClassicAssetPropertiesReferenceExistingFiles) {
    const QObject* pTheme = loadLateNightTheme(QStringLiteral("Classic"));
    ASSERT_TRUE(pTheme);

    assertLateNightAssetPropertiesReferenceExistingFiles(pTheme);
}

TEST_F(ThemeQmlTest, LateNightPaleMoonAssetPropertiesReferenceExistingFiles) {
    const QObject* pTheme = loadLateNightTheme(QStringLiteral("PaleMoon"));
    ASSERT_TRUE(pTheme);

    assertLateNightAssetPropertiesReferenceExistingFiles(pTheme);
}
