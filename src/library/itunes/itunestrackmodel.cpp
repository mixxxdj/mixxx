#include "library/itunes/itunestrackmodel.h"

#include "moc_itunestrackmodel.cpp"

ITunesTrackModel::ITunesTrackModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalTrackModel(parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.itunes",
                  "itunes_library",
                  trackSource) {
}
