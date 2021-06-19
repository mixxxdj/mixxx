#pragma once
#include <QObject>
#include <QString>

#include "mixer/playermanager.h"

namespace mixxx {
namespace skin {
namespace qml {

class QmlPlayerManagerProxy : public QObject {
    Q_OBJECT
  public:
    explicit QmlPlayerManagerProxy(
            std::shared_ptr<PlayerManager> pPlayerManager,
            QObject* parent = nullptr);

    Q_INVOKABLE QObject* getPlayer(const QString& deck);

  signals:
    void loadLocationToPlayer(const QString& location, const QString& group);

  private:
    const std::shared_ptr<PlayerManager> m_pPlayerManager;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
