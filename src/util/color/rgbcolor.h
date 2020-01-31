#pragma once

#include <QColor>
#include <QtGlobal>

#include "util/assert.h"
#include "util/optional.h"

namespace mixxx {

// A pure 24-bit 0xxRRGGBB color code with 8-bit per channel
// without an an alpha channel.
// We are using a separate typedef, because QRgb implicitly
// includes an alpha channel whereas this type does not!
typedef quint32 RgbColorCode;

// Type-safe wrapper around RgbColorCode without implicit
// range checks and no implicit validation.
//
// Apart from the assignment operator this type is immutable.
class RgbColor {
  public:
    static RgbColorCode validateCode(RgbColorCode code) {
        return code & kRgbCodeMask;
    }
    static bool isValidCode(RgbColorCode code) {
        return code == validateCode(code);
    }

    // The default constructor is not available, because there is
    // no common default value that fits all possible use cases!
    RgbColor() = delete;
    // Explicit conversion from a valid RgbColorCode.
    explicit constexpr RgbColor(RgbColorCode code)
            : m_code(code) {
        DEBUG_ASSERT(isValidCode(m_code));
    }
    // Explicit conversion from QColor.
    RgbColor(QColor anyColor, RgbColorCode codeIfInvalid)
            : m_code(anyColorToCode(anyColor, codeIfInvalid)) {
    }

    // Implicit conversion to a color code.
    operator RgbColorCode() const {
        return validateCode(m_code);
    }

    friend bool operator==(RgbColor lhs, RgbColor rhs) {
        return lhs.m_code == rhs.m_code;
    }

    typedef std::optional<RgbColor> optional_t;
    static constexpr optional_t nullopt() {
        return std::nullopt;
    }

    // Overloaded conversion functions for conveniently creating
    // std::optional<RgbColor>.
    static constexpr optional_t optional(RgbColor color) {
        return std::make_optional(color);
    }
    static constexpr optional_t optional(RgbColorCode colorCode) {
        return optional(RgbColor(colorCode));
    }
    static optional_t optional(QColor color) {
        if (color.isValid()) {
            return optional(validateCode(color.rgb()));
        } else {
            return nullopt();
        }
    }

  protected:
    // Bitmask of valid codes = 0x00RRGGBB
    static constexpr RgbColorCode kRgbCodeMask = 0x00FFFFFF;

    static RgbColorCode anyColorToCode(QColor anyColor, RgbColorCode codeIfInvalid) {
        if (anyColor.isValid()) {
            // Strip alpha channel bits!
            return validateCode(anyColor.rgb());
        } else {
            return codeIfInvalid;
        }
    }

    RgbColorCode m_code;
};

inline bool operator!=(RgbColor lhs, RgbColor rhs) {
    return !(lhs == rhs);
}

// Explicit conversion of both non-optional and optional
// RgbColor values to QColor as overloaded free functions.

inline
QColor toQColor(RgbColor color) {
    return QColor::fromRgb(color);
}

inline
QColor toQColor(std::optional<RgbColor> optional, QColor defaultColor = QColor()) {
    if (optional) {
        return toQColor(*optional);
    } else {
        return defaultColor;
    }
}

} // namespace mixxx

// Assumption: A primitive type wrapped into std::optional is
// still a primitive type.
Q_DECLARE_TYPEINFO(std::optional<mixxx::RgbColor>, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(std::optional<mixxx::RgbColor>)
