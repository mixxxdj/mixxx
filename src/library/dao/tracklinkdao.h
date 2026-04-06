#pragma once

#include <QList>

#include "library/dao/dao.h"
#include "track/trackid.h"

class TrackLink {
  public:
    enum class Type {
        Unknown = 0,
        Stem = 1,
        Duplicate = 2,
        Canonical = 3,
    };

    TrackLink()
            : id(-1),
              trackId(TrackId()),
              targetTrackId(TrackId()),
              offsetMs(0.0),
              type(Type::Unknown) {
    }

    int id;
    TrackId trackId;
    TrackId targetTrackId;
    double offsetMs;
    Type type;
};

class TrackLinkDao : public DAO {
  public:
    explicit TrackLinkDao() = default;
    ~TrackLinkDao() override = default;

    bool linkTracks(TrackId trackId,
            TrackId targetTrackId,
            double offsetMs,
            TrackLink::Type type);

    bool unlinkTracks(TrackId trackId,
            TrackId targetTrackId,
            TrackLink::Type type);

    QList<TrackLink> getLinksForTrack(TrackId trackId);
    QList<TrackLink> getLinksToTarget(TrackId targetTrackId);

    bool updateOffset(TrackId trackId,
            TrackId targetTrackId,
            TrackLink::Type type,
            double offsetMs);
};
