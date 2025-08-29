#pragma once

#include <QObject>
#include <QString>

class PlayerManager;

class BasePlayer : public QObject {
    Q_OBJECT
  public:
    BasePlayer(PlayerManager* pParent, const QString& group);
    ~BasePlayer() override = default;

    inline const QString& getGroup() const {
        return m_group;
    }

  protected:
    PlayerManager* m_pPlayerManager;
    const QString m_group;
};
