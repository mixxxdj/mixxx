#pragma once
#include <QObject>
#include <QString>

class PlayerManager;

namespace mixxx {
namespace skin {
namespace qml {

class QmlPlayerManagerProxy : public QObject {
    Q_OBJECT
  public:
    explicit QmlPlayerManagerProxy(PlayerManager* pPlayerManager, QObject* parent = nullptr);

    Q_INVOKABLE QObject* getPlayer(const QString& deck);

  signals:
    void loadLocationToPlayer(const QString& location, const QString& group);

  private:
    const PlayerManager* m_pPlayerManager;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
