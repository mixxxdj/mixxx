#pragma once
// cueinfo.h
// Created 2020-02-28 by Jan Holthuis

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
            const RgbColor::optional_t& color);

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

  private:
    CueType m_type;
    std::optional<double> m_startPositionMillis;
    std::optional<double> m_endPositionMillis;
    std::optional<int> m_hotCueIndex;
    QString m_label;
    RgbColor::optional_t m_color;
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
