#include <gtest/gtest.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <memory>

#include "library/searchqueriesstorage.h"
#include "qml/qmlconfigproxy.h"
#include "test/mixxxtest.h"

namespace {

// Tests the QML bridge the search pane in Library.qml uses for persisting
// and restoring recent searches via Mixxx.Config.getRecentSearches().
// The query string parsing itself is covered by SearchQueriesStorageTest.
class QmlRecentSearchTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::qml::QmlConfigProxy::registerUserSettings(config());
        m_engine.addImportPath(QStringLiteral(RESOURCE_FOLDER "/qml"));
    }

    QVariant loadRecentSearchesFromQml() {
        QQmlComponent component(&m_engine);
        component.setData(R"(
import QtQml
import Mixxx 1.0 as Mixxx

QtObject {
    function load() {
        return Mixxx.Config.getRecentSearches()
    }
}
)",
                QUrl::fromLocalFile(QStringLiteral(
                        RESOURCE_FOLDER "/qml/qmlrecentsearchtest.qml")));
        std::unique_ptr<QObject> pRoot(component.create());
        EXPECT_FALSE(component.isError()) << qPrintable(component.errorString());
        if (!pRoot) {
            return QVariant();
        }

        QVariant result;
        const bool invoked = QMetaObject::invokeMethod(pRoot.get(),
                "load",
                Q_RETURN_ARG(QVariant, result));
        EXPECT_TRUE(invoked);
        return result;
    }

  private:
    QQmlEngine m_engine;
};

TEST_F(QmlRecentSearchTest, ConfigQueriesRoundTripThroughQmlBridge) {
    mixxx::SearchQueriesStorage::saveQueries(config(),
            {QStringLiteral("artist:foo"), QStringLiteral("key_id:11")});

    const QVariant result = loadRecentSearchesFromQml();
    ASSERT_TRUE(result.canConvert<QVariantList>()) << qPrintable(result.toString());
    const QVariantList queries = result.toList();
    ASSERT_EQ(queries.size(), 2);
    EXPECT_QSTRING_EQ("artist:foo", queries.at(0).toString());
    EXPECT_QSTRING_EQ("key_id:11", queries.at(1).toString());
}

} // namespace
