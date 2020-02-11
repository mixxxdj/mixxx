#pragma once

#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QList>
#include <memory>

#include "util/types.h"

namespace mixxx {

// Forward declaration
class SeratoMarkersEntry;
typedef std::shared_ptr<SeratoMarkersEntry> SeratoMarkersEntryPointer;

class SeratoMarkersEntry {
  public:
    SeratoMarkersEntry(int startPosition, int endPosition, QRgb color, int type, bool isLocked)
            : m_color(color),
              m_isLocked(isLocked),
              m_endPosition(endPosition),
              m_startPosition(startPosition),
              m_type(type) {
    }
    ~SeratoMarkersEntry() = default;

    QByteArray data() const;
    static SeratoMarkersEntryPointer parse(const QByteArray& data);

    int type() const {
        return m_type;
    }

    QRgb getColor() const {
        return m_color;
    }

    bool isLocked() const {
        return m_isLocked;
    }

    int getEndPosition() const {
        return m_endPosition;
    }

    int getStartPosition() const {
        return m_startPosition;
    }

  private:
    QRgb m_color;
    bool m_isEnabled;
    bool m_isLocked;
    bool m_isSet;
    int m_endPosition;
    int m_startPosition;
    int m_type;
};

inline bool operator==(const SeratoMarkersEntry& lhs, const SeratoMarkersEntry& rhs) {
    return (lhs.data() == rhs.data());
}

inline bool operator!=(const SeratoMarkersEntry& lhs, const SeratoMarkersEntry& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoMarkersEntry& arg) {
    return dbg << "type =" << arg.type()
               << "startPosition =" << arg.getStartPosition()
               << "endPosition =" << arg.getEndPosition()
               << "color =" << arg.getColor()
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

    QByteArray data() const;

    bool isEmpty() const {
        return m_entries.isEmpty();
    }

    const QList<SeratoMarkersEntryPointer>& getEntries() const {
        return m_entries;
    }
    void setEntries(QList<SeratoMarkersEntryPointer> entries) {
        m_entries = std::move(entries);
    }

    QRgb getTrackColor() const {
        return m_trackColor;
    }
    void setTrackColor(QRgb color) {
        m_trackColor = color;
    }

  private:
    QList<SeratoMarkersEntryPointer> m_entries;
    QRgb m_trackColor;
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
