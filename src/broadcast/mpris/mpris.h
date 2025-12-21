#pragma once

#include <QDBusConnection>
#include <QObject>

#include "track/track.h"

class MediaPlayer2Player;
class MixxxMainWindow;
class PlayerManagerInterface;

class Mpris : public QObject {
    Q_OBJECT
  public:
    explicit Mpris(PlayerManagerInterface* pPlayerManager,
            UserSettingsPointer pSettings);
    ~Mpris();
    void notifyPropertyChanged(const QString& interface,
            const QString& propertyName,
            const QVariant& propertyValue);

  private:
    QDBusConnection m_busConnection;
    MediaPlayer2Player* m_pPlayer;
};
