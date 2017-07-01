#pragma once

#include "proto/keys.pb.h"

#include "track/trackid.h"
#include "track/cue.h"
#include "track/keys.h"
#include "track/keyutils.h"
#include "track/trackmetadata.h"
#include "track/playcounter.h"

#include "library/coverart.h"


namespace mixxx {

// Properties of tracks that are stored in the database.
class TrackRecord final {
    // Properties that parsed from and (optionally) written back to their
    // source, i.e. the corresponding file
    PROPERTY_SET_BYVAL_GET_BYREF(TrackMetadata,  metadata,       Metadata)

    // The unique ID of track. This value is only set once after the track
    // has been inserted or is loaded from the library DB.
    PROPERTY_SET_BYVAL_GET_BYREF(TrackId,        id,             Id)

    // TODO(uklotz): Change data type from bool to QDateTime
    //
    // Both import and export of metadata can be tracked by a single time
    // stamp, the direction doesn't matter. The value should be set to the
    // modification time stamp provided by the metadata source. This would
    // enable us to update the metadata of all tracks in the database after
    // the external metadata has been modified, i.e. if the corresponding
    // files have been modified.
    //
    // Requires a database update! We could reuse the 'header_parsed' column.
    // During migration the boolean value will be substituted with either a
    // default time stamp 1970-01-01 00:00:00.000 or NULL respectively.
    PROPERTY_SET_BYVAL_GET_BYREF(bool /*QDateTime*/, metadataSynchronized, MetadataSynchronized)

    PROPERTY_SET_BYVAL_GET_BYREF(CoverInfoRelative,  coverInfo,            CoverInfo)

    PROPERTY_SET_BYVAL_GET_BYREF(QDateTime,   dateAdded,      DateAdded)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,     fileType,       FileType)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,     url,            Url)
    PROPERTY_SET_BYVAL_GET_BYREF(PlayCounter, playCounter,    PlayCounter)
    PROPERTY_SET_BYVAL_GET_BYREF(CuePosition, cuePoint,       CuePoint)
    PROPERTY_SET_BYVAL_GET_BYREF(int,         rating,         Rating)
    PROPERTY_SET_BYVAL_GET_BYREF(bool,        bpmLocked,      BpmLocked)

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
