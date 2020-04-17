#pragma once
// cueinfo.h
// Created 2020-02-28 by Jan Holthuis

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

// DTO for Cue information without dependencies on the actual Track object
class CueInfo {
  public:
    CueInfo();
    CueInfo(CueType type,
            std::optional<double> startPositionMillis,
            std::optional<double> endPositionMillis,
            std::optional<int> hotCueNumber,
            QString label,
            RgbColor::optional_t color);

    CueType getType() const;
    void setType(CueType type);

    std::optional<double> getStartPositionMillis() const;
    void setStartPositionMillis(
            std::optional<double> positionMillis = std::nullopt);

    std::optional<double> getEndPositionMillis() const;
    void setEndPositionMillis(
            std::optional<double> positionMillis = std::nullopt);

    std::optional<int> getHotCueNumber() const;
    void setHotCueNumber(
            std::optional<int> hotCueNumber = std::nullopt);

    QString getLabel() const;
    void setLabel(
            QString label = QString());

    mixxx::RgbColor::optional_t getColor() const;
    void setColor(
            mixxx::RgbColor::optional_t color = std::nullopt);

  private:
    CueType m_type;
    std::optional<double> m_startPositionMillis;
    std::optional<double> m_endPositionMillis;
    std::optional<int> m_hotCueNumber;
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

inline QDebug operator<<(QDebug debug, const CueType& cueType) {
    switch (cueType) {
    case CueType::Invalid:
        debug << "CueType::Invalid";
        break;
    case CueType::HotCue:
        debug << "CueType::HotCue";
        break;
    case CueType::MainCue:
        debug << "CueType::MainCue";
        break;
    case CueType::Beat:
        debug << "CueType::Beat";
        break;
    case CueType::Loop:
        debug << "CueType::Loop";
        break;
    case CueType::Jump:
        debug << "CueType::Jump";
        break;
    case CueType::Intro:
        debug << "CueType::Intro";
        break;
    case CueType::Outro:
        debug << "CueType::Outro";
        break;
    case CueType::AudibleSound:
        debug << "CueType::AudibleSound";
        break;
    }
    return debug;
}

} // namespace mixxx
