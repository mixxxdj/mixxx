#pragma once

#include "track/trackid.h"
#include "track/trackmetadata.h"
#include "track/playcounter.h"

#include "library/coverart.h"


namespace mixxx {

class TrackRecord final {
    // The unique ID of track. This value is only set once after the track
    // has been inserted or is loaded from the library DB.
    PROPERTY_SET_BYVAL_GET_BYREF(TrackId,           id,             Id)

    PROPERTY_SET_BYVAL_GET_BYREF(TrackMetadata,     metadata,       Metadata)

    PROPERTY_SET_BYVAL_GET_BYREF(QDateTime,         dateAdded,      DateAdded)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,           fileType,       FileType)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,           url,            Url)
    PROPERTY_SET_BYVAL_GET_BYREF(PlayCounter,       playCounter,    PlayCounter)
    PROPERTY_SET_BYVAL_GET_BYREF(CoverInfoRelative, coverInfo,      coverInfo)
    PROPERTY_SET_BYVAL_GET_BYREF(double,            cuePoint,       CuePoint)
    PROPERTY_SET_BYVAL_GET_BYREF(int,               rating,         Rating)
    PROPERTY_SET_BYVAL_GET_BYREF(bool,              metadataParsed, MetadataParsed)
    PROPERTY_SET_BYVAL_GET_BYREF(bool,              bpmLocked,      BpmLocked)

public:
    explicit TrackRecord(TrackId id = TrackId());
    TrackRecord(TrackRecord&&) = default;
    TrackRecord(const TrackRecord&) = default;
    /*non-virtual*/ ~TrackRecord() = default;

    TrackRecord& operator=(TrackRecord&&) = default;
    TrackRecord& operator=(const TrackRecord&) = default;
};

} // namespace mixxx
