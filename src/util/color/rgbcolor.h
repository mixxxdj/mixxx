#pragma once

#include <QColor>
#include <QtGlobal>

#include <optional>

#include "util/assert.h"

namespace mixxx {

// A pure 24-bit 0xxRRGGBB color code without an alpha channel.
// We are using a separate typedef, because QRgb implicitly
// includes an alpha channel whereas this type does not!
typedef quint32 RgbColorCode;

// A thin wrapper around QColor to represent optional,
// opaque RGB colors with 8-bit per channel without an
// alpha channel. It ensures that the alpha channel is set
// to opaque for valid RGB colors and that invalid colors
// are always represented by the same, well defined QColor,
// namely QColor().
// Apart from assignment this type is immutable.
class RgbColor {
  public:
    RgbColor()
            : m_internalCode(kInvalidInternalCode) {
        DEBUG_ASSERT(!isValid());
    }
    /*non-explicit*/ RgbColor(RgbColorCode code)
            : m_internalCode(codeToInternalCode(code)) {
        DEBUG_ASSERT(isValid());
    }
    /*non-explicit*/ RgbColor(std::optional<RgbColorCode> optionalCode)
            : m_internalCode(optionalCodeToInternalCode(optionalCode)) {
        DEBUG_ASSERT(isValid() == optionalCode.has_value());
    }
    /*non-explicit*/ RgbColor(QColor anyColor)
            : m_internalCode(anyColorToInternalCode(anyColor)) {
        DEBUG_ASSERT(isValid() == anyColor.isValid());
    }

    // Implicit conversion into the corresponding QColor.
    operator QColor() const {
        return internalCodeToColor(m_internalCode);
    }

    // Checks if the color is valid or represents "no color",
    // i.e. is missing or undefined. If the corresponding color
    // code is stored in a database then the colum value is NULL.
    bool isValid() const {
        return m_internalCode == (m_internalCode & kRgbCodeMask);
    }

    // Returns the corresponding, optional RGB color code.
    std::optional<RgbColorCode> optionalCode() const {
        return internalCodeToOptionalCode(m_internalCode);
    }

    friend bool operator==(const RgbColor& lhs, const RgbColor& rhs) {
        return lhs.m_internalCode == rhs.m_internalCode;
    }

  private:
    static const RgbColorCode kRgbCodeMask = 0x00FFFFFF;
    static const RgbColorCode kAlphaCodeMask = 0xFF000000;
    static const RgbColorCode kInvalidInternalCode = 0xFFFFFFFF;

    static RgbColorCode codeToInternalCode(RgbColorCode code) {
        return code & kRgbCodeMask;
    }
    static RgbColorCode optionalCodeToInternalCode(std::optional<RgbColorCode> optionalCode) {
        if (optionalCode.has_value()) {
            return codeToInternalCode(optionalCode.value());
        } else {
            return kInvalidInternalCode;
        }
    }

    static RgbColorCode anyColorToInternalCode(QColor anyColor) {
        if (anyColor.isValid()) {
            return anyColor.rgb() & kRgbCodeMask;
        } else {
            return kInvalidInternalCode;
        }
    }

    std::optional<RgbColorCode> internalCodeToOptionalCode(RgbColorCode internalCode) const {
        if (internalCode == (internalCode & kRgbCodeMask)) {
            return std::make_optional(m_internalCode);
        } else {
            return std::nullopt;
        }
    }

    QColor internalCodeToColor(RgbColorCode internalCode) const {
        if (internalCode == (internalCode & kRgbCodeMask)) {
            return QRgb(m_internalCode | kAlphaCodeMask);
        } else {
            return QColor();
        }
    }

    RgbColorCode m_internalCode;
};

inline bool operator!=(const RgbColor& lhs, const RgbColor& rhs) {
    return !(lhs == rhs);
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::RgbColor, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::RgbColor)
