#pragma once

#include <QColor>

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
    RgbColor() {
        DEBUG_ASSERT(!isValid());
    }
    // Implicit conversion constructors without normalization.
    // Only use these conversion constructors if the argument
    // matches an RgbColor! Otherwise a debug assertion will
    // be triggered.
    /*non-explicit*/ RgbColor(RgbColorCode code)
            : m_color(toQColor(code)) {
        DEBUG_ASSERT(m_color == normalizeQColor(m_color));
    }
    /*non-explicit*/ RgbColor(QColor color)
            : m_color(color) {
        DEBUG_ASSERT(m_color == normalizeQColor(m_color));
    }

    // Check that the provided color code is valid.
    static bool isValidCode(RgbColorCode code) {
        return code == (code & kRgbCodeMask);
    }

    // Explicit conversions with normalization.
    // Use these static functions instead of the conversion
    // constructors to ensure that the resulting RgbColor is
    // well defined.
    static RgbColor fromCode(RgbColorCode code) {
        return RgbColor(normalizeCode(code));
    }
    static RgbColor fromQColor(QColor color) {
        return RgbColor(normalizeQColor(color));
    }

    // Implicit conversion into the corresponding QColor.
    operator QColor() const {
        return m_color;
    }

    // Checks if the color is valid or represents "no color",
    // i.e. is missing or undefined. If the corresponding color
    // code is stored in a database then the colum value is NULL.
    bool isValid() const {
        return m_color.isValid();
    }

    // Returns the corresponding RGB color code of a valid color.
    // Validity is a prerequisite and only checked by a debug
    // assertion.
    RgbColorCode code() const {
        DEBUG_ASSERT(isValid());
        return toCode(m_color);
    }

    // Returns the corresponding RGB color code of a valid color
    // or the provided color code if the color is not valid.
    RgbColorCode codeOr(RgbColorCode codeIfNotValid) const {
        if (isValid()) {
            return code();
        } else {
            return codeIfNotValid;
        }
    }

    friend bool operator==(const RgbColor& lhs, const RgbColor& rhs) {
        return lhs.m_color == rhs.m_color;
    }

  private:
    static const RgbColorCode kRgbCodeMask = 0x00FFFFFF;
    static const RgbColorCode kAlphaCodeMask = 0xFF000000;

    static QColor toQColor(RgbColorCode code) {
        DEBUG_ASSERT(isValidCode(code));
        return QColor(code | kAlphaCodeMask);
    }
    static RgbColorCode normalizeCode(RgbColorCode code) {
        return code & kRgbCodeMask;
    }
    static QColor normalizeQColor(QColor color) {
        if (color.isValid()) {
            return toQColor(toCode(color));
        } else {
            return QColor();
        }
    }
    static RgbColorCode toCode(QColor color) {
        DEBUG_ASSERT(color.isValid());
        return normalizeCode(color.rgb());
    }

    QColor m_color;
};

inline bool operator!=(const RgbColor& lhs, const RgbColor& rhs) {
    return !(lhs == rhs);
}

} // namespace mixxx
