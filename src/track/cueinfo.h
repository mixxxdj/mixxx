#pragma once

#include <QFlags>

#include "audio/signalinfo.h"
#include "util/color/rgbcolor.h"
#include "util/optional.h"

namespace mixxx {

enum class CueType {
    Invalid = 0,
    HotCue = 1,
    MainCue = 2,
    Beat = 3, // unused (what is this for?)
    Loop = 4,
    Jump = 5,
    Intro = 6,
    Outro = 7,
    AudibleSound = 8, // range that covers beginning and end of audible
                      // sound; not shown to user
};

enum class CueFlag {
    None = 0,
    /// Currently only used when importing locked loops from Serato Metadata.
    Locked = 1,
};
Q_DECLARE_FLAGS(CueFlags, CueFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(CueFlags);

/// Hot cues are sequentially indexed starting with kFirstHotCueIndex (inclusive)
static constexpr int kFirstHotCueIndex = 0;

// DTO for Cue information without dependencies on the actual Track object
class CueInfo {
  public:
    CueInfo();
    CueInfo(CueType type,
            const std::optional<double>& startPositionMillis,
            const std::optional<double>& endPositionMillis,
            const std::optional<int>& hotCueIndex,
            QString label,
            const RgbColor::optional_t& color,
            CueFlags flags = CueFlag::None);

    CueType getType() const;
    void setType(CueType type);

    std::optional<double> getStartPositionMillis() const;
    void setStartPositionMillis(
            const std::optional<double>& positionMillis = std::nullopt);

    std::optional<double> getEndPositionMillis() const;
    void setEndPositionMillis(
            const std::optional<double>& positionMillis = std::nullopt);

    std::optional<int> getHotCueIndex() const;
    void setHotCueIndex(
            const std::optional<int>& hotCueIndex = std::nullopt);

    QString getLabel() const;
    void setLabel(
            const QString& label = QString());

    mixxx::RgbColor::optional_t getColor() const;
    void setColor(
            const mixxx::RgbColor::optional_t& color = std::nullopt);

    CueFlags flags() const {
        return m_flags;
    }

    /// Set flags for the cue.
    ///
    /// These flags are currently only set during Serato cue import and *not*
    /// saved in the Database (only used for roundtrip testing purposes).
    void setFlags(CueFlags flags) {
        m_flags = flags;
    }

    /// Checks if the `CueFlag::Locked` flag is set for this cue.
    bool isLocked() const {
        return m_flags.testFlag(CueFlag::Locked);
    }

  private:
    CueType m_type;
    std::optional<double> m_startPositionMillis;
    std::optional<double> m_endPositionMillis;
    std::optional<int> m_hotCueIndex;
    QString m_label;
    RgbColor::optional_t m_color;
    CueFlags m_flags;
};

bool operator==(
        const CueInfo& lhs,
        const CueInfo& rhs);

inline bool operator!=(
        const CueInfo& lhs,
        const CueInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const CueType& cueType);
QDebug operator<<(QDebug debug, const CueInfo& cueInfo);

} // namespace mixxx
