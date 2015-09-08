#ifndef TRACKREF_H
#define TRACKREF_H


#include <ostream>
#include <utility>

#include <QDateTime>
#include <QFileInfo>
#include <QString>
#include <QtDebug>

#include "track/trackid.h"
#include "util/assert.h"


// A track in the library is identified by a location and an id.
// The location is mandatory to identify the file, whereas the id
// only exists after the track has been inserted into the database.
//
// The logical reference to the track's file is referred to as its
// "location". The location of each track must be unique inside the
// library, i.e. each location identifies a single track. The location
// contains an absolute path to the file.
//
// Different locations might reference the same file on disk. In order
// to track which files are in use at runtime we also need to reference
// tracks by their "canonical location". Each file has a unique canonical
// location that is determined at runtime. The canonical location is
// a transient property that may vary between different executions of the
// application, but must not change during a single execution. It is
// obtained by removing all redundancies from and resolving all symbolic
// links in the location.
//
// NOTE(uklotzde): I've decided not to add any comparison operators for
// this class, because technical and logical equality differs depending
// on the use case!
class TrackRef {
public:
    // Functions for extracting commonly used information from QFileInfo.
    static QString toLocation(const QFileInfo& fileInfo) {
        return fileInfo.absoluteFilePath();
    }
    static QString toCanonicalLocation(const QFileInfo& fileInfo) {
        return fileInfo.canonicalFilePath();
    }
    static QString toDirectory(const QFileInfo& fileInfo) {
        return fileInfo.absolutePath();
    }
    static QString toFileName(const QFileInfo& fileInfo) {
        return fileInfo.fileName();
    }
    static quint64 toFileSize(const QFileInfo& fileInfo) {
        return fileInfo.size();
    }
    static QDateTime toFileCreatedAt(const QFileInfo& fileInfo) {
        return fileInfo.created();
    }
    static QDateTime toFileLastModifiedAt(const QFileInfo& fileInfo) {
        return fileInfo.lastModified();
    }

    TrackRef() {
        DEBUG_ASSERT(verifyConsistency());
    }
    // Conversion from QFileInfo
    explicit TrackRef(
            const QFileInfo& fileInfo,
            TrackId id = TrackId())
        : m_location(toLocation(fileInfo)),
          m_canonicalLocation(toCanonicalLocation(fileInfo)),
          m_id(std::move(id)) {
        DEBUG_ASSERT(verifyConsistency());
    }
    // Create a copy of an existing TrackRef and set the TrackId.
    // This is equivalent to invoking the regular copy constructor
    // followed by a call to setId().
    TrackRef(
            const TrackRef& other,
            TrackId id)
        : m_location(other.m_location),
          m_canonicalLocation(other.m_canonicalLocation),
          m_id(std::move(id)) {
        DEBUG_ASSERT(!other.hasId() || (other.getId() == id));
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
    // files. Like the id it can only be set once.
    const QString& getCanonicalLocation() const {
        return m_canonicalLocation;
    }
    bool hasCanonicalLocation() const {
        return !getCanonicalLocation().isEmpty();
    }
    void setCanonicalLocation(QString canonicalLocation) {
        DEBUG_ASSERT(!hasCanonicalLocation() ||
                (getCanonicalLocation() == getCanonicalLocation()));
        m_canonicalLocation = std::move(canonicalLocation);
        DEBUG_ASSERT(verifyConsistency());
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
    void setId(TrackId id) {
        DEBUG_ASSERT(!hasId() || (getId() == id));
        m_id = std::move(id);
        DEBUG_ASSERT(verifyConsistency());
    }

    // Conversion of a TrackRef into the corresponding QFileInfo.
    QFileInfo createFileInfo() const;

private:
    bool verifyConsistency() const;

    QString m_location;
    QString m_canonicalLocation;
    TrackId m_id;
};

Q_DECLARE_METATYPE(TrackRef)

std::ostream& operator<<(std::ostream& os, const TrackRef& trackRef);

QDebug operator<<(QDebug qDebug, const TrackRef& trackRef);


#endif // TRACKREF_H
