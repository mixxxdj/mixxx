#ifndef MEDIAPLAYER2PLAYLIST_H
#define MEDIAPLAYER2PLAYLIST_H

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusObjectPath>
#include <QStringList>

// this implements the Version 2.2 of 
// MPRIS D-Bus Interface Specification
// org.mpris.MediaPlayer2.Playlists
// http://specifications.freedesktop.org/mpris-spec/2.2/


typedef struct {
    QDBusObjectPath o;
    QString s1;
    QString s2;
} Playlist;
Q_DECLARE_METATYPE(Playlist)

typedef struct {
    bool valid;
    Playlist playlist;
} MaybePlaylist;


class MediaPlayer2Playlists : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Playlists")
    Q_PROPERTY(uint PlaylistCount READ playlistCount)
    Q_PROPERTY(QStringList Orderings READ orderings)
    Q_PROPERTY(MaybePlaylist ActivePlaylist READ activePlaylist)

  public:
    MediaPlayer2Playlists(QObject* parent = 0);
    virtual ~MediaPlayer2Playlists();

    uint playlistCount() const;
    QStringList orderings() const;
    MaybePlaylist activePlaylist() const;

  public slots:
    void ActivatePlaylist(const QDBusObjectPath& PlaylistId);
    QList<Playlist> GetPlaylists(uint index, uint maxCount, const QString& order,
                                 bool reverseOrder);

  signals:
    void PlaylistChanged(const Playlist& playlist);
};

#endif // MEDIAPLAYER2PLAYLIST_H
