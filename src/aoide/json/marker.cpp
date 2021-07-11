#include "aoide/json/marker.h"

#include "track/keyutils.h"
#include "util/assert.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide Marker");

} // anonymous namespace

namespace aoide {

namespace json {

//static
std::optional<CueMarker> CueMarker::fromCueInfo(
        const mixxx::CueInfo& cueInfo) {
    VERIFY_OR_DEBUG_ASSERT(
            cueInfo.getStartPositionMillis() ||
            cueInfo.getEndPositionMillis()) {
        kLogger.warning()
                << "Ignoring cue of type"
                << static_cast<int>(cueInfo.getType())
                << "with undefined start/in and end/out positions";
        return std::nullopt;
    }
    CueMarker cueMarker;
    cueMarker.setType(cueInfo.getType());
    cueMarker.setSlotIndex(cueInfo.getHotCueIndex());
    cueMarker.setInPositionMillis(cueInfo.getStartPositionMillis());
    cueMarker.setOutPositionMillis(cueInfo.getEndPositionMillis());
    if (cueInfo.getType() == mixxx::CueType::Loop) {
        cueMarker.setOutMode(kOutModeLoop);
    }
    cueMarker.setLabel(cueInfo.getLabel());
    cueMarker.setColor(cueInfo.getColor());
    return cueMarker;
}

mixxx::CueType CueMarker::type() const {
    return static_cast<mixxx::CueType>(
            m_jsonObject.value(QLatin1String("bankIndex"))
                    .toInt(static_cast<int>(mixxx::CueType::Invalid)));
}

void CueMarker::setType(mixxx::CueType type) {
    m_jsonObject.insert(QLatin1String("bankIndex"), static_cast<int>(type));
}

std::optional<int> CueMarker::slotIndex() const {
    const auto jsonValue = m_jsonObject.value(QLatin1String("slotIndex"));
    if (jsonValue.isUndefined() || jsonValue.isNull()) {
        return std::nullopt;
    }
    return jsonValue.toInt();
}

void CueMarker::setSlotIndex(std::optional<int> slotIndex) {
    if (slotIndex) {
        m_jsonObject.insert(QLatin1String("slotIndex"), *slotIndex);
    } else {
        m_jsonObject.remove(QLatin1String("slotIndex"));
    }
}

std::optional<double> CueMarker::inPositionMillis() const {
    return getOptionalDouble(QLatin1String("inPositionMs"));
}

void CueMarker::setInPositionMillis(std::optional<double> inPositionMillis) {
    putOptional(QLatin1String("inPositionMs"), inPositionMillis);
}

std::optional<double> CueMarker::outPositionMillis() const {
    return getOptionalDouble(QLatin1String("outPositionMs"));
}

void CueMarker::setOutPositionMillis(std::optional<double> outPositionMillis) {
    putOptional(QLatin1String("outPositionMs"), outPositionMillis);
}

int CueMarker::outMode() const {
    const auto jsonValue = m_jsonObject.value(QLatin1String("outMode"));
    if (jsonValue.isNull() || jsonValue.isUndefined()) {
        return kOutModeDefault;
    }
    return jsonValue.toInt(kOutModeDefault);
}

void CueMarker::setOutMode(int outMode) {
    if (outMode != kOutModeDefault) {
        m_jsonObject.insert(QLatin1String("outMode"), outMode);
    } else {
        m_jsonObject.remove(QLatin1String("outMode"));
    }
}

QString CueMarker::label() const {
    return m_jsonObject.value(QLatin1String("label")).toString();
}

void CueMarker::setLabel(QString label) {
    putOptionalNonEmpty(QLatin1String("label"), std::move(label));
}

mixxx::RgbColor::optional_t CueMarker::color() const {
    return mixxx::RgbColor::fromQString(
            m_jsonObject
                    .value(QLatin1String("color"))
                    .toObject()
                    .value(QLatin1String("rgb"))
                    .toString());
}

void CueMarker::setColor(mixxx::RgbColor::optional_t color) {
    if (color) {
        m_jsonObject.insert(QLatin1String("color"),
                QJsonObject{
                        {QLatin1String("rgb"), mixxx::RgbColor::toQString(*color)},
                });
    } else {
        m_jsonObject.remove(QLatin1String("color"));
    }
}

CueMarkerVector CueMarkers::fromJson() const {
    CueMarkerVector markers;
    markers.reserve(m_jsonArray.size());
    for (const auto& elem : m_jsonArray) {
        markers.append(CueMarker(elem.toObject()));
    }
    return markers;
}

} // namespace json

} // namespace aoide
