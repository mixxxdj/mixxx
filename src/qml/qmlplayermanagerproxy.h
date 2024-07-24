#pragma once
#include <QObject>
#include <QString>
#include <QtQml>

#include "mixer/playermanager.h"
#include "qml/qmlplayerproxy.h"

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
    static void registerPlayerManager(std::shared_ptr<PlayerManager> pPlayerManager) {
        s_pPlayerManager = std::move(pPlayerManager);
    }

  private:
    static inline std::shared_ptr<PlayerManager> s_pPlayerManager;

    const std::shared_ptr<PlayerManager> m_pPlayerManager;
};

} // namespace qml
} // namespace mixxx
