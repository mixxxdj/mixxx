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

class SeratoBeatGridNonTerminalMarker;
class SeratoBeatGridTerminalMarker;

typedef std::shared_ptr<SeratoBeatGridNonTerminalMarker> SeratoBeatGridNonTerminalMarkerPointer;
typedef std::shared_ptr<SeratoBeatGridTerminalMarker> SeratoBeatGridTerminalMarkerPointer;

class SeratoBeatGridNonTerminalMarker {
  public:
    SeratoBeatGridNonTerminalMarker(float positionMillis, quint32 beatsTillNextMarker)
            : m_positionMillis(positionMillis),
              m_beatTillNextMarker(beatsTillNextMarker) {
    }
    ~SeratoBeatGridNonTerminalMarker() = default;

    QByteArray dumpID3() const;
    static SeratoBeatGridNonTerminalMarkerPointer parseID3(const QByteArray&);

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

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridNonTerminalMarker& arg) {
    return dbg << "SeratoBeatGridNonTerminalMarker"
               << "PositionMillis =" << arg.positionMillis()
               << "BeatTillNextMarker = " << arg.beatsTillNextMarker();
}

class SeratoBeatGridTerminalMarker {
  public:
    SeratoBeatGridTerminalMarker(float positionMillis, float bpm)
            : m_positionMillis(positionMillis),
              m_bpm(bpm) {
    }
    ~SeratoBeatGridTerminalMarker() = default;

    QByteArray dumpID3() const;
    static mixxx::SeratoBeatGridTerminalMarkerPointer parseID3(const QByteArray&);

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

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridTerminalMarker& arg) {
    return dbg << "SeratoBeatGridTerminalMarker"
               << "PositionMillis =" << arg.positionMillis()
               << "BPM =" << arg.bpm();
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
    explicit SeratoBeatGrid(
            SeratoBeatGridTerminalMarkerPointer pTerminalMarker,
            QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers)
            : m_pTerminalMarker(pTerminalMarker),
              m_nonTerminalMarkers(std::move(nonTerminalMarkers)),
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
        return !m_pTerminalMarker && m_nonTerminalMarkers.isEmpty();
    }

    const QList<SeratoBeatGridNonTerminalMarkerPointer>& nonTerminalMarkers() const {
        return m_nonTerminalMarkers;
    }
    void setNonTerminalMarkers(QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers) {
        m_nonTerminalMarkers = nonTerminalMarkers;
    }

    const SeratoBeatGridTerminalMarkerPointer terminalMarker() const {
        return m_pTerminalMarker;
    }
    void setTerminalMarker(SeratoBeatGridTerminalMarkerPointer pTerminalMarker) {
        m_pTerminalMarker = pTerminalMarker;
    }

    quint8 footer() const {
        return m_footer;
    }
    void setFooter(quint8 footer) {
        m_footer = footer;
    }

  private:
    SeratoBeatGridTerminalMarkerPointer m_pTerminalMarker;
    QList<SeratoBeatGridNonTerminalMarkerPointer> m_nonTerminalMarkers;
    quint8 m_footer;
};

inline bool operator==(const SeratoBeatGrid& lhs, const SeratoBeatGrid& rhs) {
    return (lhs.terminalMarker() == rhs.terminalMarker() &&
            lhs.nonTerminalMarkers() == rhs.nonTerminalMarkers());
}

inline bool operator!=(const SeratoBeatGrid& lhs, const SeratoBeatGrid& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoBeatGrid& arg) {
    // TODO: Improve debug output
    return dbg << "number of markers ="
               << (arg.nonTerminalMarkers().length() +
                          (arg.terminalMarker() ? 1 : 0));
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::SeratoBeatGrid, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::SeratoBeatGrid)
