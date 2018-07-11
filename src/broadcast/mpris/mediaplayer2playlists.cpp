#include <QApplication>

#include "mediaplayer2playlists.h"

MediaPlayer2Playlists::MediaPlayer2Playlists(QObject *parent)
    : QDBusAbstractAdaptor(parent) {
}

MediaPlayer2Playlists::~MediaPlayer2Playlists() {
}

uint MediaPlayer2Playlists::playlistCount() const {
    return 0;
}

QStringList MediaPlayer2Playlists::orderings() const {
    QStringList orderings;
    return orderings;
}

MaybePlaylist MediaPlayer2Playlists::activePlaylist() const {
    MaybePlaylist activePlaylist;
    activePlaylist.valid = false;
    return activePlaylist;
}

void MediaPlayer2Playlists::ActivatePlaylist(const QDBusObjectPath& playlistId) {
    Q_UNUSED(playlistId);
}

QList<Playlist> MediaPlayer2Playlists::GetPlaylists(uint index,
                                                    uint maxCount,
                                                    const QString& order,
                                                    bool reverseOrder) {
    Q_UNUSED(index);
    Q_UNUSED(maxCount);
    Q_UNUSED(order);
    Q_UNUSED(reverseOrder);

    QList<Playlist> playlists;
    return playlists;
}
