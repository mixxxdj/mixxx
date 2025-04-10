#pragma once

#include <QDebug>
#include <QRgb>

#include "util/color/rgbcolor.h"

namespace mixxx {

class SeratoStoredColor {
  public:
    static constexpr RgbColor kNoColor = RgbColor(0xFFFFFF);
    static constexpr RgbColor kFixedLoopColor = RgbColor(0x27AAE1);
    static constexpr RgbColor kFixedUnsetColor = RgbColor(0x000000);

    explicit constexpr SeratoStoredColor(RgbColor::code_t code)
            : m_color(code) {
    }

    friend bool operator==(SeratoStoredColor lhs, SeratoStoredColor rhs) {
        return lhs.m_color == rhs.m_color;
    }

    friend QDebug operator<<(QDebug dbg, SeratoStoredColor color) {
        return dbg << color.m_color;
    }

    QRgb toQRgb() const {
        return m_color;
    }

  protected:
    RgbColor m_color;
};

class SeratoStoredTrackColor final : public SeratoStoredColor {
  public:
    using SeratoStoredColor::SeratoStoredColor;

    RgbColor::optional_t toDisplayedColor() const;
    static SeratoStoredTrackColor fromDisplayedColor(RgbColor::optional_t color);
};

class SeratoStoredHotcueColor final : public SeratoStoredColor {
  public:
    using SeratoStoredColor::SeratoStoredColor;

    RgbColor::optional_t toDisplayedColor() const;
    static SeratoStoredHotcueColor fromDisplayedColor(RgbColor::optional_t color);
};

} // namespace mixxx
