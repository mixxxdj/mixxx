#pragma once

#include <QVector>

#include "aoide/json/json.h"
#include "track/cueinfo.h"
#include "util/color/rgbcolor.h"

namespace aoide {

namespace json {

class CueMarker : public Object {
  public:
    static std::optional<CueMarker> fromCueInfo(
            const mixxx::CueInfo& cueInfo);

    explicit CueMarker(
            QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~CueMarker() override = default;

    // mixxx::CueType is mapped to the corresponding bank index
    mixxx::CueType type() const;
    void setType(
            mixxx::CueType type = mixxx::CueType::Invalid);

    std::optional<int> slotIndex() const;
    void setSlotIndex(std::optional<int> slotIndex = std::nullopt);

    std::optional<double> inPositionMillis() const;
    void setInPositionMillis(
            std::optional<double> inPositionMillis = std::nullopt);

    std::optional<double> outPositionMillis() const;
    void setOutPositionMillis(
            std::optional<double> outPositionMillis = std::nullopt);

    static constexpr int kOutModeCont = 0;
    static constexpr int kOutModeStop = 1;
    static constexpr int kOutModeLoop = 2;
    static constexpr int kOutModeNext = 3;
    static constexpr int kOutModeDefault = kOutModeCont;

    int outMode() const;
    void setOutMode(int outMode = kOutModeDefault);

    QString label() const;
    void setLabel(
            QString label = QString());

    mixxx::RgbColor::optional_t color() const;
    void setColor(
            mixxx::RgbColor::optional_t color = mixxx::RgbColor::nullopt());
};

typedef QVector<CueMarker> CueMarkerVector;

class CueMarkers : public Array {
  public:
    explicit CueMarkers(
            QJsonArray jsonArray = QJsonArray())
            : Array(std::move(jsonArray)) {
    }

    void append(CueMarker cueMarker) {
        m_jsonArray.append(cueMarker.intoQJsonValue());
    }

    CueMarkerVector fromJson() const;
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::CueMarker);
Q_DECLARE_METATYPE(aoide::json::CueMarkerVector);
Q_DECLARE_METATYPE(aoide::json::CueMarkers);
