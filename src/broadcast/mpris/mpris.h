#ifndef MPRIS_H
#define MPRIS_H

#include <QDBusConnection>
#include <QObject>

#include "broadcast/mpris/mediaplayer2player.h"
#include "track/track.h"

class MixxxMainWindow;

class Mpris : public QObject {
    Q_OBJECT
  public:
    explicit Mpris(MixxxMainWindow* mixxx);
    ~Mpris();
    void broadcastCurrentTrack();

  private:
    void notifyPropertyChanged(const QString& interface,
            const QString& propertyName,
            const QVariant& propertyValue);

    QDBusConnection m_busConnection;
    MediaPlayer2Player* m_pPlayer;
};

#endif // MPRIS_H
