#pragma once

#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QList>
#include <memory>

#include "track/cueinfo.h"
#include "track/taglib/trackmetadata_file.h"
#include "util/types.h"

namespace mixxx {

class SeratoMarkersEntry;
typedef std::shared_ptr<SeratoMarkersEntry> SeratoMarkersEntryPointer;

class SeratoMarkersEntry {
  public:
    /// We didn't encounter other type IDs as those listed here (e.g. "2") yet.
    /// Apparently these are not used.
    enum class TypeId : int {
        /// Used for unset cue points
        Unknown = 0,
        /// Used for set cue points
        Cue = 1,
        /// Used for saved loops (both set and unset ones)
        Loop = 3,
    };

    SeratoMarkersEntry(
            bool hasStartPosition,
            int startPosition,
            bool hasEndPosition,
            int endPosition,
            RgbColor color,
            int type,
            bool isLocked)
            : m_color(color),
              m_hasStartPosition(hasStartPosition),
              m_hasEndPosition(hasEndPosition),
              m_isLocked(isLocked),
              m_startPosition(startPosition),
              m_endPosition(endPosition),
              m_type(type) {
    }
    ~SeratoMarkersEntry() = default;

    QByteArray dumpID3() const;
    QByteArray dumpMP4() const;

    static SeratoMarkersEntryPointer parseID3(const QByteArray& data);
    static SeratoMarkersEntryPointer parseMP4(const QByteArray& data);

    int type() const {
        return m_type;
    }

    SeratoMarkersEntry::TypeId typeId() const {
        SeratoMarkersEntry::TypeId typeId = SeratoMarkersEntry::TypeId::Unknown;
        switch (static_cast<SeratoMarkersEntry::TypeId>(type())) {
        case SeratoMarkersEntry::TypeId::Unknown:
            // This seems to be an unset Hotcue (i.e. without a position)
        case SeratoMarkersEntry::TypeId::Cue:
            typeId = SeratoMarkersEntry::TypeId::Cue;
            break;
        case SeratoMarkersEntry::TypeId::Loop:
            typeId = SeratoMarkersEntry::TypeId::Loop;
            break;
        }
        return typeId;
    }

    RgbColor getColor() const {
        return m_color;
    }

    bool isLocked() const {
        return m_isLocked;
    }

    bool hasStartPosition() const {
        return m_hasStartPosition;
    }

    quint32 getStartPosition() const {
        return m_startPosition;
    }

    bool hasEndPosition() const {
        return m_hasEndPosition;
    }

    quint32 getEndPosition() const {
        return m_endPosition;
    }

  private:
    RgbColor m_color;
    bool m_hasStartPosition;
    bool m_hasEndPosition;
    ;
    bool m_isLocked;
    quint32 m_startPosition;
    quint32 m_endPosition;
    int m_type;
};

inline bool operator==(const SeratoMarkersEntry& lhs, const SeratoMarkersEntry& rhs) {
    return (lhs.dumpID3() == rhs.dumpID3());
}

inline bool operator!=(const SeratoMarkersEntry& lhs, const SeratoMarkersEntry& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoMarkersEntry& arg) {
    dbg << "type =" << arg.type();
    if (arg.hasStartPosition()) {
        dbg << "startPosition =" << arg.getStartPosition();
    }
    if (arg.hasEndPosition()) {
        dbg << "endPosition =" << arg.getEndPosition();
    }
    return dbg << "color =" << arg.getColor()
               << "isLocked =" << arg.isLocked();
}

/// DTO for storing information from the SeratoMarkers_ tags used by the Serato
/// DJ Pro software.
///
/// This class includes functions for formatting and parsing SeratoMarkers_
/// metadata according to the specification:
/// https://github.com/Holzhaus/serato-tags/blob/master/docs/serato_markers_.md
class SeratoMarkers final {
  public:
    SeratoMarkers() = default;

    /// Parse a binary Serato representation of the "Markers_" data from a
    /// `QByteArray` and write the results to the `SeratoMarkers` instance.
    /// The `fileType` parameter determines the exact format of the data being
    /// used.
    static bool parse(
            SeratoMarkers* seratoMarkers,
            const QByteArray& data,
            taglib::FileType fileType);

    /// Create a binary Serato representation of the "Markers_" data suitable
    /// for `fileType` and dump it into a `QByteArray`. The content of that
    /// byte array can be used for round-trip tests or written to the
    /// appropriate tag to make it accessible to Serato.
    QByteArray dump(taglib::FileType fileType) const;

    bool isEmpty() const {
        return m_entries.isEmpty() && !m_trackColor;
    }

    const QList<SeratoMarkersEntryPointer>& getEntries() const {
        return m_entries;
    }
    void setEntries(QList<SeratoMarkersEntryPointer> entries) {
        m_entries = entries;
    }

    RgbColor::optional_t getTrackColor() const {
        return m_trackColor;
    }
    void setTrackColor(RgbColor::optional_t color) {
        m_trackColor = color;
    }

    QList<CueInfo> getCues() const;
    void setCues(const QList<CueInfo>& cueInfos);

  private:
    static bool parseID3(
            SeratoMarkers* seratoMarkers,
            const QByteArray& data);
    static bool parseMP4(
            SeratoMarkers* seratoMarkers,
            const QByteArray& base64EncodedData);

    QByteArray dumpID3() const;
    QByteArray dumpMP4() const;

    QList<SeratoMarkersEntryPointer> m_entries;
    RgbColor::optional_t m_trackColor;
};

inline bool operator==(const SeratoMarkers& lhs, const SeratoMarkers& rhs) {
    return (lhs.getEntries() == rhs.getEntries());
}

inline bool operator!=(const SeratoMarkers& lhs, const SeratoMarkers& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoMarkers& arg) {
    return dbg << "entries =" << arg.getEntries().length();
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::SeratoMarkers, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::SeratoMarkers)
