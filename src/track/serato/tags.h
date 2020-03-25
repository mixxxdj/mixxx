#pragma once

#include "track/serato/markers.h"
#include "track/serato/markers2.h"

namespace mixxx {

// DTO for storing information from the SeratoMarkers_/2 tags used by the
// Serato DJ Pro software.
//
class SeratoTags final {
  public:
    static constexpr RgbColor kDefaultTrackColor = RgbColor(0xFF9999);
    static constexpr RgbColor kDefaultCueColor = RgbColor(0xCC0000);

    SeratoTags() = default;

    static RgbColor::optional_t storedToDisplayedTrackColor(RgbColor color);
    static RgbColor displayedToStoredTrackColor(RgbColor::optional_t color);
    static RgbColor storedToDisplayedSeratoDJProCueColor(RgbColor color);
    static RgbColor displayedToStoredSeratoDJProCueColor(RgbColor color);
    static double findTimingOffsetMillis(const QString& filePath);

    bool isEmpty() const {
        return m_seratoMarkers.isEmpty() && m_seratoMarkers2.isEmpty();
    }

    bool parseMarkers(const QByteArray& data) {
        return SeratoMarkers::parse(&m_seratoMarkers, data);
    }

    bool parseMarkers2(const QByteArray& data) {
        return SeratoMarkers2::parse(&m_seratoMarkers2, data);
    }

    QByteArray dumpMarkers() const {
        return m_seratoMarkers.dump();
    }

    QByteArray dumpMarkers2() const {
        return m_seratoMarkers2.dump();
    }

    QList<CueInfo> getCues(const QString& filePath) const;

    RgbColor::optional_t getTrackColor() const;
    bool isBpmLocked() const;

  private:
    SeratoMarkers m_seratoMarkers;
    SeratoMarkers2 m_seratoMarkers2;
};

inline bool operator==(const SeratoTags& lhs, const SeratoTags& rhs) {
    // FIXME: Find a more efficient way to do this
    return (lhs.dumpMarkers() == rhs.dumpMarkers() && lhs.dumpMarkers2() == rhs.dumpMarkers2());
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
