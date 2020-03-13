#pragma once

#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QList>
#include <memory>

#include "util/color/rgbcolor.h"
#include "util/types.h"

namespace mixxx {

class SeratoMarkersEntry;
typedef std::shared_ptr<SeratoMarkersEntry> SeratoMarkersEntryPointer;

class SeratoMarkersEntry {
  public:
    enum class TypeId {
        Unknown,
        Cue,
        Loop,
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

    QByteArray dump() const;
    static SeratoMarkersEntryPointer parse(const QByteArray& data);

    int type() const {
        return m_type;
    }

    SeratoMarkersEntry::TypeId typeId() const {
        SeratoMarkersEntry::TypeId typeId = SeratoMarkersEntry::TypeId::Unknown;
        switch (type()) {
        case 0: // This seems to be an unset Hotcue (i.e. without a position)
        case 1: // Hotcue
            typeId = SeratoMarkersEntry::TypeId::Cue;
            break;
        case 3: // Saved Loop
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
    bool m_isSet;
    quint32 m_startPosition;
    quint32 m_endPosition;
    int m_type;
};

inline bool operator==(const SeratoMarkersEntry& lhs, const SeratoMarkersEntry& rhs) {
    return (lhs.dump() == rhs.dump());
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

// DTO for storing information from the SeratoMarkers_ tags used by the Serato
// DJ Pro software.
//
// Parsing & Formatting
// --------------------
// This class includes functions for formatting and parsing SeratoMarkers_
// metadata according to the specification:
// https://github.com/Holzhaus/serato-tags/blob/master/docs/serato_markers_.md
//
class SeratoMarkers final {
  public:
    SeratoMarkers() = default;
    explicit SeratoMarkers(QList<SeratoMarkersEntryPointer> entries)
            : m_entries(std::move(entries)) {
    }

    static bool parse(SeratoMarkers* seratoMarkers, const QByteArray& data);

    QByteArray dump() const;

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

  private:
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
