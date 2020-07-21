#pragma once

#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QList>
#include <memory>

#include "track/cueinfo.h"
#include "track/taglib/trackmetadata_file.h"
#include "util/assert.h"
#include "util/types.h"

namespace mixxx {

class SeratoBeatGridEntry;

typedef std::shared_ptr<SeratoBeatGridEntry> SeratoBeatGridEntryPointer;

class SeratoBeatGridEntry {
  public:
    enum class Type {
        NonTerminal,
        Terminal,
    };

    SeratoBeatGridEntry(Type type)
            : m_type(type) {
    }
    ~SeratoBeatGridEntry() = default;

    virtual QByteArray dumpID3() const = 0;

    Type type() const {
        return m_type;
    }

  private:
    Type m_type;
};

inline bool operator==(const SeratoBeatGridEntry& lhs, const SeratoBeatGridEntry& rhs) {
    return (lhs.type() == rhs.type() && lhs.dumpID3() == rhs.dumpID3());
}

inline bool operator!=(const SeratoBeatGridEntry& lhs, const SeratoBeatGridEntry& rhs) {
    return !(lhs == rhs);
}

class SeratoBeatGridNonTerminalEntry : public SeratoBeatGridEntry {
  public:
    SeratoBeatGridNonTerminalEntry(float positionMillis, quint32 beatsTillNextMarker)
            : SeratoBeatGridEntry(SeratoBeatGridEntry::Type::NonTerminal),
              m_positionMillis(positionMillis),
              m_beatTillNextMarker(beatsTillNextMarker) {
    }
    ~SeratoBeatGridNonTerminalEntry() = default;

    virtual QByteArray dumpID3() const;
    static SeratoBeatGridEntryPointer parseID3(const QByteArray&);

    float positionMillis() const {
        return m_positionMillis;
    }

    float beatsTillNextMarker() const {
        return m_beatTillNextMarker;
    }

  private:
    float m_positionMillis;
    quint32 m_beatTillNextMarker;
};

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridNonTerminalEntry& arg) {
    return dbg << "SeratoBeatGridNonTerminalEntry"
               << "PositionMillis =" << arg.positionMillis()
               << "BeatTillNextMarker = " << arg.beatsTillNextMarker();
}

class SeratoBeatGridTerminalEntry : public SeratoBeatGridEntry {
  public:
    SeratoBeatGridTerminalEntry(float positionMillis, float bpm)
            : SeratoBeatGridEntry(SeratoBeatGridEntry::Type::Terminal),
              m_positionMillis(positionMillis),
              m_bpm(bpm) {
    }
    ~SeratoBeatGridTerminalEntry() = default;

    virtual QByteArray dumpID3() const;
    static mixxx::SeratoBeatGridEntryPointer parseID3(const QByteArray&);

    float positionMillis() const {
        return m_positionMillis;
    }

    float bpm() const {
        return m_bpm;
    }

  private:
    float m_positionMillis;
    float m_bpm;
};

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridTerminalEntry& arg) {
    return dbg << "SeratoBeatGridTerminalEntry"
               << "PositionMillis =" << arg.positionMillis()
               << "BPM =" << arg.bpm();
}

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridEntry& arg) {
    switch (arg.type()) {
    case SeratoBeatGridEntry::Type::NonTerminal:
        return dbg << static_cast<const SeratoBeatGridNonTerminalEntry&>(arg);
    case SeratoBeatGridEntry::Type::Terminal:
        return dbg << static_cast<const SeratoBeatGridTerminalEntry&>(arg);
    default:
        DEBUG_ASSERT(!"Invalid SeratoBeatGridEntry type!");
        return dbg;
    }
}

// DTO for storing information from the SeratoBeatGrid tags used by the Serato
// DJ Pro software.
//
// Parsing & Formatting
// --------------------
// This class includes functions for formatting and parsing SeratoBeatGrid
// metadata according to the specification:
// https://github.com/Holzhaus/serato-tags/blob/master/docs/serato_beatgrid.md
//
class SeratoBeatGrid final {
  public:
    SeratoBeatGrid() = default;
    explicit SeratoBeatGrid(QList<SeratoBeatGridEntryPointer> entries)
            : m_entries(std::move(entries)),
              m_footer(0) {
    }

    static bool parse(
            SeratoBeatGrid* seratoBeatGrid,
            const QByteArray& data,
            taglib::FileType fileType);
    static bool parseID3(
            SeratoBeatGrid* seratoBeatGrid,
            const QByteArray& data);

    QByteArray dump(taglib::FileType fileType) const;
    QByteArray dumpID3() const;

    bool isEmpty() const {
        return m_entries.isEmpty();
    }

    const QList<SeratoBeatGridEntryPointer>& getEntries() const {
        return m_entries;
    }
    void setEntries(QList<SeratoBeatGridEntryPointer> entries) {
        m_entries = entries;
    }

    quint8 footer() const {
        return m_footer;
    }
    void setFooter(quint8 footer) {
        m_footer = footer;
    }

  private:
    QList<SeratoBeatGridEntryPointer> m_entries;
    quint8 m_footer;
};

inline bool operator==(const SeratoBeatGrid& lhs, const SeratoBeatGrid& rhs) {
    return (lhs.getEntries() == rhs.getEntries());
}

inline bool operator!=(const SeratoBeatGrid& lhs, const SeratoBeatGrid& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoBeatGrid& arg) {
    return dbg << "entries =" << arg.getEntries().length();
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::SeratoBeatGrid, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::SeratoBeatGrid)
