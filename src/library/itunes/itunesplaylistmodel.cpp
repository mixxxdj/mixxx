#include "library/itunes/itunesplaylistmodel.h"

#include "library/baseexternalplaylistmodel.h"

ITunesPlaylistModel::ITunesPlaylistModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.itunes_playlist",
                  "itunes_playlists",
                  "itunes_playlist_tracks",
                  trackSource) {
}

void ITunesPlaylistModel::initSortColumnMapping() {
    // TODO
}
