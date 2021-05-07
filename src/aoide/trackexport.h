#pragma once

#include <QList>

#include "aoide/json/collection.h"
#include "aoide/json/track.h"
#include "track/cue.h"

namespace mixxx {

class FileInfo;
class TrackRecord;

} // namespace mixxx

namespace aoide {

json::Track exportTrack(
        const json::MediaSourceConfig& mediaSourceConfig,
        const mixxx::FileInfo& trackFile,
        const mixxx::TrackRecord& trackRecord,
        const QList<CuePointer>& cuePoints);

} // namespace aoide
