#include <QApplication>

#include "mediaplayer2tracklist.h"

MediaPlayer2TrackList::MediaPlayer2TrackList(QObject* parent)
    : QDBusAbstractAdaptor(parent) {
}

MediaPlayer2TrackList::~MediaPlayer2TrackList() {
}

TrackIds MediaPlayer2TrackList::tracks() const {
    TrackIds tracks;
    return tracks;
}

bool MediaPlayer2TrackList::canEditTracks() const {
    return false;
}

TrackMetadata MediaPlayer2TrackList::GetTracksMetadata(
        const TrackIds& tracks) const {
    Q_UNUSED(tracks);

    TrackMetadata metadata;
    return metadata;
}

void MediaPlayer2TrackList::AddTrack(const QString& uri,
                                     const QDBusObjectPath& afterTrack,
                                     bool setAsCurrent) {
    Q_UNUSED(uri);
    Q_UNUSED(afterTrack);
    Q_UNUSED(setAsCurrent);
}

void MediaPlayer2TrackList::RemoveTrack(const QDBusObjectPath& trackId) {
    Q_UNUSED(trackId);
}

void MediaPlayer2TrackList::GoTo(const QDBusObjectPath& trackId) {
    Q_UNUSED(trackId);
}




