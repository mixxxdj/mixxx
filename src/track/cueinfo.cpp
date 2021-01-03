#include "track/cueinfo.h"

#include "util/assert.h"

namespace mixxx {

CueInfo::CueInfo()
        : m_type(CueType::Invalid),
          m_startPositionMillis(std::nullopt),
          m_endPositionMillis(std::nullopt),
          m_hotCueIndex(std::nullopt),
          m_color(std::nullopt) {
}

CueInfo::CueInfo(
        CueType type,
        const std::optional<double>& startPositionMillis,
        const std::optional<double>& endPositionMillis,
        const std::optional<int>& hotCueIndex,
        QString label,
        const mixxx::RgbColor::optional_t& color)
        : m_type(type),
          m_startPositionMillis(startPositionMillis),
          m_endPositionMillis(endPositionMillis),
          m_hotCueIndex(hotCueIndex),
          m_label(std::move(label)),
          m_color(color) {
}

CueType CueInfo::getType() const {
    return m_type;
}

void CueInfo::setType(CueType type) {
    m_type = type;
}

void CueInfo::setStartPositionMillis(const std::optional<double>& positionMillis) {
    m_startPositionMillis = positionMillis;
}

std::optional<double> CueInfo::getStartPositionMillis() const {
    return m_startPositionMillis;
}

void CueInfo::setEndPositionMillis(const std::optional<double>& positionMillis) {
    m_endPositionMillis = positionMillis;
}

std::optional<double> CueInfo::getEndPositionMillis() const {
    return m_endPositionMillis;
}

std::optional<int> CueInfo::getHotCueIndex() const {
    return m_hotCueIndex;
}

void CueInfo::setHotCueIndex(const std::optional<int>& hotCueIndex) {
    DEBUG_ASSERT(!hotCueIndex || *hotCueIndex >= kFirstHotCueIndex);
    m_hotCueIndex = hotCueIndex;
}

QString CueInfo::getLabel() const {
    return m_label;
}

void CueInfo::setLabel(const QString& label) {
    m_label = label;
}

RgbColor::optional_t CueInfo::getColor() const {
    return m_color;
}

void CueInfo::setColor(const RgbColor::optional_t& color) {
    m_color = color;
}

bool operator==(
        const CueInfo& lhs,
        const CueInfo& rhs) {
    return lhs.getType() == rhs.getType() &&
            lhs.getStartPositionMillis() == rhs.getStartPositionMillis() &&
            lhs.getEndPositionMillis() == rhs.getEndPositionMillis() &&
            lhs.getHotCueIndex() == rhs.getHotCueIndex() &&
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
            << ", index=" << cueInfo.getHotCueIndex()
            << ", label=" << cueInfo.getLabel()
            << ", color=" << cueInfo.getColor()
            << "]";
    return debug;
}

} // namespace mixxx
