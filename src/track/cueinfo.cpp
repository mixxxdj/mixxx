#include "track/cueinfo.h"

#include "util/assert.h"

namespace mixxx {

CueInfo::CueInfo()
        : m_type(CueType::Invalid),
          m_startPositionMillis(std::nullopt),
          m_endPositionMillis(std::nullopt),
          m_hotCueNumber(std::nullopt),
          m_color(std::nullopt) {
}

CueInfo::CueInfo(
        CueType type,
        std::optional<double> startPositionMillis,
        std::optional<double> endPositionMillis,
        std::optional<int> hotCueNumber,
        QString label,
        mixxx::RgbColor::optional_t color)
        : m_type(type),
          m_startPositionMillis(startPositionMillis),
          m_endPositionMillis(endPositionMillis),
          m_hotCueNumber(hotCueNumber),
          m_label(label),
          m_color(color) {
}

CueType CueInfo::getType() const {
    return m_type;
}

void CueInfo::setType(CueType type) {
    m_type = type;
}

void CueInfo::setStartPositionMillis(std::optional<double> positionMillis) {
    m_startPositionMillis = positionMillis;
}

std::optional<double> CueInfo::getStartPositionMillis() const {
    return m_startPositionMillis;
}

void CueInfo::setEndPositionMillis(std::optional<double> positionMillis) {
    m_endPositionMillis = positionMillis;
}

std::optional<double> CueInfo::getEndPositionMillis() const {
    return m_endPositionMillis;
}

std::optional<int> CueInfo::getHotCueNumber() const {
    return m_hotCueNumber;
}

void CueInfo::setHotCueNumber(std::optional<int> hotCueNumber) {
    m_hotCueNumber = hotCueNumber;
}

QString CueInfo::getLabel() const {
    return m_label;
}

void CueInfo::setLabel(QString label) {
    m_label = label;
}

RgbColor::optional_t CueInfo::getColor() const {
    return m_color;
}

void CueInfo::setColor(RgbColor::optional_t color) {
    m_color = color;
}

bool operator==(
        const CueInfo& lhs,
        const CueInfo& rhs) {
    return lhs.getType() == rhs.getType() &&
            lhs.getStartPositionMillis() == rhs.getStartPositionMillis() &&
            lhs.getEndPositionMillis() == rhs.getEndPositionMillis() &&
            lhs.getHotCueNumber() == rhs.getHotCueNumber() &&
            lhs.getLabel() == rhs.getLabel() &&
            lhs.getColor() == rhs.getColor();
}

QDebug operator<<(QDebug debug, const CueType& cueType) {
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

QDebug operator<<(QDebug debug, const CueInfo& cueInfo) {
    debug.nospace()
            << "CueInfo["
            << "type=" << cueInfo.getType()
            << ", startPos=" << cueInfo.getStartPositionMillis()
            << ", endPos=" << cueInfo.getEndPositionMillis()
            << ", number=" << cueInfo.getHotCueNumber()
            << ", label=" << cueInfo.getLabel()
            << ", color=" << cueInfo.getColor()
            << "]";
    return debug;
}

} // namespace mixxx
