#include "track/cueinfo.h"

#include "util/assert.h"

namespace mixxx {

namespace {

void assertEndPosition(
        CueType type,
        std::optional<double> endPositionMillis) {
    Q_UNUSED(endPositionMillis);
    switch (type) {
    case CueType::HotCue:
    case CueType::MainCue:
        DEBUG_ASSERT(!endPositionMillis);
        break;
    case CueType::Loop:
    case CueType::Jump:
    case CueType::N60dBSound:
        DEBUG_ASSERT(endPositionMillis);
        break;
    case CueType::Intro:
    case CueType::Outro:
    case CueType::Invalid:
        break;
    case CueType::Beat: // unused
    default:
        DEBUG_ASSERT(!"Unknown Loop Type");
    }
}

} // namespace

CueInfo::CueInfo()
        : m_type(CueType::Invalid),
          m_startPositionMillis(std::nullopt),
          m_endPositionMillis(std::nullopt),
          m_hotCueIndex(std::nullopt),
          m_color(std::nullopt),
          m_stem1vol(std::nullopt),
          m_stem2vol(std::nullopt),
          m_stem3vol(std::nullopt),
          m_stem4vol(std::nullopt),
          m_flags(CueFlag::None) {
}

CueInfo::CueInfo(
        CueType type,
        const std::optional<double>& startPositionMillis,
        const std::optional<double>& endPositionMillis,
        const std::optional<int>& hotCueIndex,
        QString label,
        const mixxx::RgbColor::optional_t& color,
        const std::optional<int>& stem1vol,
        const std::optional<int>& stem2vol,
        const std::optional<int>& stem3vol,
        const std::optional<int>& stem4vol,
        CueFlags flags)
        : m_type(type),
          m_startPositionMillis(startPositionMillis),
          m_endPositionMillis(endPositionMillis),
          m_hotCueIndex(hotCueIndex),
          m_label(std::move(label)),
          m_color(color),
          m_stem1vol(stem1vol),
          m_stem2vol(stem2vol),
          m_stem3vol(stem3vol),
          m_stem4vol(stem4vol),
          m_flags(flags) {
    assertEndPosition(type, endPositionMillis);
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
    assertEndPosition(m_type, m_endPositionMillis);
}

std::optional<double> CueInfo::getEndPositionMillis() const {
    return m_endPositionMillis;
}

std::optional<int> CueInfo::getHotCueIndex() const {
    return m_hotCueIndex;
}

void CueInfo::setHotCueIndex(int hotCueIndex) {
    m_hotCueIndex = hotCueIndex;
}

std::optional<int> CueInfo::getStem1vol() const {
    return m_stem1vol;
}

std::optional<int> CueInfo::getStem2vol() const {
    return m_stem2vol;
}

std::optional<int> CueInfo::getStem3vol() const {
    return m_stem3vol;
}

std::optional<int> CueInfo::getStem4vol() const {
    return m_stem4vol;
}

void CueInfo::setStem1vol(int stem1vol) {
    m_stem1vol = stem1vol;
}

void CueInfo::setStem2vol(int stem2vol) {
    m_stem2vol = stem2vol;
}

void CueInfo::setStem3vol(int stem3vol) {
    m_stem3vol = stem3vol;
}

void CueInfo::setStem4vol(int stem4vol) {
    m_stem4vol = stem4vol;
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
            lhs.getColor() == rhs.getColor() &&
            lhs.getStem1vol() == rhs.getStem1vol() &&
            lhs.getStem2vol() == rhs.getStem2vol() &&
            lhs.getStem3vol() == rhs.getStem3vol() &&
            lhs.getStem4vol() == rhs.getStem4vol();
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
    case CueType::N60dBSound:
        debug << "CueType::N60dBSound";
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
            << ", stem1vol=" << cueInfo.getStem1vol()
            << ", stem2vol=" << cueInfo.getStem2vol()
            << ", stem3vol=" << cueInfo.getStem3vol()
            << ", stem4vol=" << cueInfo.getStem4vol()
            << ", flags=" << cueInfo.flags()
            << "]";
    return debug;
}

} // namespace mixxx
