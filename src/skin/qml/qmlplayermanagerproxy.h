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

  private:
    const PlayerManager* m_pPlayerManager;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
