#pragma once
#include <QObject>
#include <QString>
#include <QtQml>

#include "mixer/playermanager.h"

namespace mixxx {
namespace qml {

class QmlPlayerManagerProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlayerManager)
    QML_SINGLETON
  public:
    explicit QmlPlayerManagerProxy(
            std::shared_ptr<PlayerManager> pPlayerManager,
            QObject* parent = nullptr);

    Q_INVOKABLE QObject* getPlayer(const QString& deck);
    Q_INVOKABLE void loadLocationIntoNextAvailableDeck(const QString& location, bool play = false);
    Q_INVOKABLE void loadLocationUrlIntoNextAvailableDeck(
            const QUrl& locationUrl, bool play = false);
    Q_INVOKABLE void loadLocationToPlayer(
            const QString& location, const QString& group, bool play = false);

    static QmlPlayerManagerProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static inline QmlPlayerManagerProxy* s_pInstance = nullptr;

  private:
    static inline QJSEngine* s_pJsEngine = nullptr;
    const std::shared_ptr<PlayerManager> m_pPlayerManager;
};

} // namespace qml
} // namespace mixxx
