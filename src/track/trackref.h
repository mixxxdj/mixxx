#ifndef TRACKREF_H
#define TRACKREF_H


#include <QFileInfo>

#include "track/trackid.h"


// A track in the library is identified by a location and an id.
// The location is mandatory to identify the file, whereas the id
// only exists after the track has been inserted into the database.
//
// This class is intended to be used as a simple, almost immutable
// value object. Only the id can be set once.
class TrackRef {
public:
    static QString location(const QFileInfo& fileInfo) {
        return fileInfo.absoluteFilePath();
    }
    static QString canonicalLocation(const QFileInfo& fileInfo) {
        return fileInfo.canonicalFilePath();
    }

    TrackRef() {
        DEBUG_ASSERT(verifyConsistency());
    }
    // Conversion from QFileInfo with an optional TrackId
    explicit TrackRef(
            const QFileInfo& fileInfo,
            TrackId id = TrackId())
        : m_location(location(fileInfo)),
          m_canonicalLocation(canonicalLocation(fileInfo)),
          m_id(id) {
        DEBUG_ASSERT(verifyConsistency());
    }
    // Create a copy of an existing TrackRef, but overwrite the
    // TrackId with a custom value.
    TrackRef(
            const TrackRef& other,
            TrackId id)
        : m_location(other.m_location),
          m_canonicalLocation(other.m_canonicalLocation),
          m_id(id) {
        DEBUG_ASSERT(verifyConsistency());
    }

    // The human-readable identifier of a track in Mixxx. The location is
    // immutable and the starting point for accessing a track's file.
    const QString& getLocation() const {
        return m_location;
    }
    bool hasLocation() const {
        return !getLocation().isEmpty();
    }

    // The unique identifier of a track's file at runtime and used
    // for caching. The canonical location is empty for inexistent
    // files.
    const QString& getCanonicalLocation() const {
        return m_canonicalLocation;
    }
    bool hasCanonicalLocation() const {
        return !getCanonicalLocation().isEmpty();
    }

    // The primary key of a track in the Mixxx library. The id must only
    // be set once after inserting into or after loading from the database.
    // Tracks that have not been stored in the database don't have an id.
    const TrackId& getId() const {
        return m_id;
    }
    bool hasId() const {
        return getId().isValid();
    }

    bool isValid() const {
        return hasId() || hasCanonicalLocation();
    }

private:
    bool verifyConsistency() const;

    QString m_location;
    QString m_canonicalLocation;
    TrackId m_id;
};

inline
bool operator==(const TrackRef& lhs, const TrackRef& rhs) {
    return (lhs.getId() == rhs.getId()) &&
            (lhs.getLocation() == rhs.getLocation()) &&
            (lhs.getCanonicalLocation() == rhs.getCanonicalLocation());
}

inline
bool operator!=(const TrackRef& lhs, const TrackRef& rhs) {
    return !(lhs == rhs);
}

Q_DECLARE_METATYPE(TrackRef)

std::ostream& operator<<(std::ostream& os, const TrackRef& trackRef);

QDebug operator<<(QDebug debug, const TrackRef& trackRef);


#endif // TRACKREF_H
