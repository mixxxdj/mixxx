#pragma once

#include "proto/keys.pb.h"

#include "track/trackid.h"
#include "track/keys.h"
#include "track/keyutils.h"
#include "track/trackmetadata.h"
#include "track/playcounter.h"

#include "library/coverart.h"


namespace mixxx {

// Properties of tracks that are stored in the database.
class TrackRecord final {
    // The unique ID of track. This value is only set once after the track
    // has been inserted or is loaded from the library DB.
    PROPERTY_SET_BYVAL_GET_BYREF(TrackId,           id,             Id)

    // Properties that are parsed from and (optionally) written back into files
    PROPERTY_SET_BYVAL_GET_BYREF(TrackMetadata,     metadata,       Metadata)

    PROPERTY_SET_BYVAL_GET_BYREF(QDateTime,         dateAdded,      DateAdded)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,           fileType,       FileType)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,           url,            Url)
    PROPERTY_SET_BYVAL_GET_BYREF(PlayCounter,       playCounter,    PlayCounter)
    PROPERTY_SET_BYVAL_GET_BYREF(double,            cuePoint,       CuePoint)
    PROPERTY_SET_BYVAL_GET_BYREF(int,               rating,         Rating)
    PROPERTY_SET_BYVAL_GET_BYREF(bool,              metadataParsed, MetadataParsed)
    PROPERTY_SET_BYVAL_GET_BYREF(bool,              bpmLocked,      BpmLocked)

    PROPERTY_SET_BYVAL_GET_BYREF(CoverInfoRelative, coverInfo,      CoverInfo)

public:
    explicit TrackRecord(TrackId id = TrackId());
    TrackRecord(TrackRecord&&) = default;
    TrackRecord(const TrackRecord&) = default;
    /*non-virtual*/ ~TrackRecord() = default;

    TrackRecord& operator=(TrackRecord&&) = default;
    TrackRecord& operator=(const TrackRecord&) = default;

    void setKeys(const Keys& keys);
    void resetKeys() {
        setKeys(Keys());
    }
    const Keys& getKeys() const {
        return m_keys;
    }

    mixxx::track::io::key::ChromaticKey getGlobalKey() const {
        if (getKeys().isValid()) {
            return getKeys().getGlobalKey();
        } else {
            return mixxx::track::io::key::INVALID;
        }
    }
    bool updateGlobalKey(
            mixxx::track::io::key::ChromaticKey key,
            mixxx::track::io::key::Source keySource);

    QString getGlobalKeyText() const {
        return KeyUtils::getGlobalKeyText(getKeys());
    }
    bool updateGlobalKeyText(
            const QString& keyText,
            mixxx::track::io::key::Source keySource);

private:
    Keys m_keys;
};

} // namespace mixxx
