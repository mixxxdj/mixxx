#pragma once

#include "audio/signalinfo.h"
#include "track/beatsimporter.h"
#include "track/cueinfoimporter.h"
#include "track/serato/beatgrid.h"
#include "track/serato/markers.h"
#include "track/serato/markers2.h"

namespace mixxx {

/// DTO for storing information from the SeratoMarkers_/2 tags used by the
/// Serato DJ Pro software.
class SeratoTags final {
  public:
    static constexpr RgbColor kDefaultTrackColor = RgbColor(0xFF9999);
    static constexpr RgbColor kDefaultCueColor = RgbColor(0xCC0000);

    SeratoTags() = default;

    static RgbColor::optional_t storedToDisplayedTrackColor(RgbColor color);
    static RgbColor displayedToStoredTrackColor(RgbColor::optional_t color);
    static RgbColor storedToDisplayedSeratoDJProCueColor(RgbColor color);
    static RgbColor displayedToStoredSeratoDJProCueColor(RgbColor color);
    static double guessTimingOffsetMillis(
            const QString& filePath, const audio::SignalInfo& signalInfo);

    bool isEmpty() const {
        return m_seratoBeatGrid.isEmpty() && m_seratoMarkers.isEmpty() &&
                m_seratoMarkers2.isEmpty();
    }

    bool parseBeatGrid(const QByteArray& data, taglib::FileType fileType) {
        return SeratoBeatGrid::parse(&m_seratoBeatGrid, data, fileType);
    }

    bool parseMarkers(const QByteArray& data, taglib::FileType fileType) {
        return SeratoMarkers::parse(&m_seratoMarkers, data, fileType);
    }

    bool parseMarkers2(const QByteArray& data, taglib::FileType fileType) {
        return SeratoMarkers2::parse(&m_seratoMarkers2, data, fileType);
    }

    QByteArray dumpBeatGrid(taglib::FileType fileType) const {
        return m_seratoBeatGrid.dump(fileType);
    }

    QByteArray dumpMarkers(taglib::FileType fileType) const {
        return m_seratoMarkers.dump(fileType);
    }

    QByteArray dumpMarkers2(taglib::FileType fileType) const {
        return m_seratoMarkers2.dump(fileType);
    }

    CueInfoImporterPointer importCueInfos() const;
    BeatsImporterPointer importBeats() const;

    QList<CueInfo> getCueInfos() const;
    void setCueInfos(const QList<CueInfo>& cueInfos, double timingOffset = 0);

    RgbColor::optional_t getTrackColor() const;
    void setTrackColor(const RgbColor::optional_t& color);

    bool isBpmLocked() const;
    void setBpmLocked(bool bpmLocked);

  private:
    SeratoBeatGrid m_seratoBeatGrid;
    SeratoMarkers m_seratoMarkers;
    SeratoMarkers2 m_seratoMarkers2;
};

inline bool operator==(const SeratoTags& lhs, const SeratoTags& rhs) {
    // FIXME: Find a more efficient way to do this
    return (lhs.dumpBeatGrid(taglib::FileType::MP3) ==
                    rhs.dumpBeatGrid(taglib::FileType::MP3) &&
            lhs.dumpMarkers(taglib::FileType::MP3) ==
                    rhs.dumpMarkers(taglib::FileType::MP3) &&
            lhs.dumpMarkers2(taglib::FileType::MP3) ==
                    rhs.dumpMarkers2(taglib::FileType::MP3));
}

inline bool operator!=(const SeratoTags& lhs, const SeratoTags& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoTags& arg) {
    Q_UNUSED(arg);

    // TODO
    return dbg << "SeratoTags";
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::SeratoTags, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::SeratoTags)
