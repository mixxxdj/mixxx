#include "track/cueinfo.h"

#include "util/assert.h"

namespace {
const QString kDefaultLabel = QStringLiteral(""); // empty string, not null
} // anonymous namespace

namespace mixxx {

CueInfo::CueInfo()
        : m_type(CueType::Invalid),
          m_startPositionMillis(std::nullopt),
          m_endPositionMillis(std::nullopt),
          m_hotCueNumber(std::nullopt),
          m_label(kDefaultLabel),
          m_color(std::nullopt) {
    DEBUG_ASSERT(!m_label.isNull());
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
    DEBUG_ASSERT(!m_label.isNull());
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
    DEBUG_ASSERT(!label.isNull());
    m_label = label;
}

RgbColor::optional_t CueInfo::getColor() const {
    return m_color;
}

void CueInfo::setColor(RgbColor::optional_t color) {
    m_color = color;
}

} // namespace mixxx
