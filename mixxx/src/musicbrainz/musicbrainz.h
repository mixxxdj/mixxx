#pragma once

#include <QMetaType>
#include <QString>
#include <QUuid>

#include "util/duration.h"

namespace mixxx {

namespace musicbrainz {

struct TrackRelease final {
    TrackRelease() = default;
    TrackRelease(const TrackRelease&) = default;
    TrackRelease(TrackRelease&&) = default;
    TrackRelease& operator=(const TrackRelease&) = default;
    TrackRelease& operator=(TrackRelease&&) = default;
    ~TrackRelease() = default;

    QUuid artistId;
    QUuid albumArtistId;
    QUuid albumReleaseId;
    QUuid recordingId;
    QUuid releaseGroupId;
    QUuid trackReleaseId;

    QString title;
    QString artist;
    QString albumTitle;
    QString albumArtist;
    QString releaseGroupType;
    QString mediumFormat;
    QString trackNumber;
    QString trackTotal;
    QString discNumber;
    QString discTotal;
    QString date;

    Duration duration;
};

void registerMetaTypesOnce();

} // namespace musicbrainz

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::musicbrainz::TrackRelease);

Q_DECLARE_METATYPE(QList<mixxx::musicbrainz::TrackRelease>);
