#ifndef MPRIS_H
#define MPRIS_H

#include <QObject>
#include <QDBusConnection>
#include "track/track.h"
#include "broadcast/mpris/mediaplayer2player.h"

class MixxxMainWindow;

class Mpris : public QObject
{
    Q_OBJECT
  public:
    explicit Mpris(MixxxMainWindow* mixxx);
    ~Mpris();
    void broadcastCurrentTrack();
  private:

    void notifyPropertyChanged(const QString &interface,
                               const QString &propertyName,
                               const QVariant &propertyValue);

    QDBusConnection m_busConnection;
    MediaPlayer2Player *m_pPlayer;
};

#endif // MPRIS_H
