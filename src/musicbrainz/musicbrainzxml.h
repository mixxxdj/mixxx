#pragma once

#include <QList>
#include <QPair>
#include <QXmlStreamReader>

#include "musicbrainz/musicbrainz.h"

namespace mixxx {

namespace musicbrainz {

struct Error final {
    Error();
    explicit Error(QXmlStreamReader& reader);
    Error(const Error&) = default;
    Error(Error&&) = default;
    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;
    ~Error() = default;

    int code;
    QString message;
};

QPair<QList<TrackRelease>, bool> parseRecordings(QXmlStreamReader& reader);

} // namespace musicbrainz

} // namespace mixxx
