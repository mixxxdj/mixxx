#ifndef MEDIAPLAYER2TRACKLIST_H
#define MEDIAPLAYER2TRACKLIST_H

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusObjectPath>
#include <QStringList>

// this implements the Version 2.2 of 
// MPRIS D-Bus Interface Specification
// org.mpris.MediaPlayer2.TrackList
// http://specifications.freedesktop.org/mpris-spec/2.2/

typedef QList<QVariantMap> TrackMetadata;
Q_DECLARE_METATYPE(TrackMetadata)
typedef QList<QDBusObjectPath> TrackIds;

class MediaPlayer2TrackList : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.TrackList")
    Q_PROPERTY(TrackIds Tracks READ tracks)
    Q_PROPERTY(bool CanEditTracks READ canEditTracks)

  public:
    MediaPlayer2TrackList(QObject* parent = 0);
    virtual ~MediaPlayer2TrackList();

    TrackIds tracks() const;
    bool canEditTracks() const;

  public slots:
    TrackMetadata GetTracksMetadata(const TrackIds& tracks) const;
    void AddTrack(const QString& uri, const QDBusObjectPath& afterTrack, bool setAsCurrent);
    void RemoveTrack(const QDBusObjectPath& trackId);
    void GoTo(const QDBusObjectPath& trackId);

  signals:
    void TrackListReplaced(const TrackIds& tracks, const QDBusObjectPath& currentTrack);
    void TrackAdded(const TrackMetadata& metadata, const QDBusObjectPath& afterTrack);
    void TrackRemoved(const QDBusObjectPath& trackId);
    void TrackMetadataChanged(const QDBusObjectPath& trackId, const TrackMetadata& metadata);
};

#endif // MEDIAPLAYER2TRACKLIST_H
