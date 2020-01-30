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

// A thin wrapper around 24-bit opaque RGB values that includes
// a single undefined state. The undefined state could be used
// to represent missing or transparent values.
// This class might be used as a "hub" for the conversion between
// RgbColorCode, std::optional<RgbColorCode>, and QColor. For
// this particulare use case it's only purpose is to store an
// intermediate representation before the internal value that
// has been converted from the source type is finally converted
// into the desired target type.
// std::optional<RgbColorCode> is equivalent, RgbColorCode is
// less versatile, and QColor is more versatile.
// Apart from the assignment operator this type is immutable.
class RgbColor {
  public:
    RgbColor()
            : m_internalCode(kUndefinedInternalCode) {
        DEBUG_ASSERT(!*this);
    }
    // Implicit conversions from RgbColorCode
    /*implicit*/ RgbColor(RgbColorCode code)
            : m_internalCode(codeToInternalCode(code)) {
        DEBUG_ASSERT(*this);
    }
    /*implicit*/ RgbColor(std::optional<RgbColorCode> optionalCode)
            : m_internalCode(optionalCodeToInternalCode(optionalCode)) {
        DEBUG_ASSERT(static_cast<bool>(*this) == static_cast<bool>(optionalCode));
    }
    // Explicit conversion from QColor
    explicit RgbColor(QColor anyColor)
            : m_internalCode(anyColorToInternalCode(anyColor)) {
        DEBUG_ASSERT(static_cast<bool>(*this) == anyColor.isValid());
    }

    // Check if the color is defined.
    operator bool() const noexcept {
        return m_internalCode != kUndefinedInternalCode;
    }

    // Explicit conversion to an optional RGB color code.
    // NOTE: An implicit conversion operator conflicts with the
    // implicit conversion constructors of std::optional and
    // causes undefined(?) or at least unexpected behavior!!
    std::optional<RgbColorCode> optional() const {
        if (m_internalCode == (m_internalCode & kRgbCodeMask)) {
            // Defined
            DEBUG_ASSERT(*this);
            return std::make_optional(m_internalCode);
        } else {
            // Undefined
            DEBUG_ASSERT(!*this);
            return std::nullopt;
        }
    }

    // Explicit conversion into the corresponding QColor.
    // Optionally the QColor that represents an undefined
    // value could be provided.
    QColor toQColor(QColor undefinedColor = QColor()) const {
        if (m_internalCode == (m_internalCode & kRgbCodeMask)) {
            // Defined
            DEBUG_ASSERT(*this);
            return QColor::fromRgb(m_internalCode);
        } else {
            // Undefined
            DEBUG_ASSERT(!*this);
            return undefinedColor;
        }
    }

    friend bool operator==(const RgbColor& lhs, const RgbColor& rhs) {
        return lhs.m_internalCode == rhs.m_internalCode;
    }

  private:
    static const RgbColorCode kRgbCodeMask = 0x00FFFFFF;
    static const RgbColorCode kUndefinedInternalCode = 0xFFFFFFFF;

    static RgbColorCode codeToInternalCode(RgbColorCode code) {
        // The unused bits of an RgbColorCode should never be set.
        DEBUG_ASSERT(code == (code & kRgbCodeMask));
        // This normalization should not be necessary, just in case.
        return code & kRgbCodeMask;
    }
    static RgbColorCode optionalCodeToInternalCode(std::optional<RgbColorCode> optionalCode) {
        if (optionalCode) {
            return codeToInternalCode(*optionalCode);
        } else {
            return kUndefinedInternalCode;
        }
    }

    static RgbColorCode anyColorToInternalCode(QColor anyColor) {
        if (anyColor.isValid()) {
            // Strip alpha channel bits
            return anyColor.rgb() & kRgbCodeMask;
        } else {
            return kUndefinedInternalCode;
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
