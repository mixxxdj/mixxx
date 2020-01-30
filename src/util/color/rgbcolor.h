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
class RgbColor {
  public:
    // The default constructor is not available, because there is
    // no common default value that fits all possible use cases!
    RgbColor() = delete;
    // Explicit conversion from RgbColorCode.
    explicit constexpr RgbColor(RgbColorCode code)
            : m_code(code) {
    }
    // Explicit conversion from QColor.
    RgbColor(QColor anyColor, RgbColorCode codeIfInvalid)
            : m_code(anyColorToCode(anyColor, codeIfInvalid)) {
    }

    // Explicit conversion to a valid color code.
    RgbColorCode validCode() const {
        return validateCode(m_code);
    }

    // Explicit conversion into the corresponding QColor.
    QColor validQColor() const {
        return QColor::fromRgb(validCode());
    }

    friend bool operator==(RgbColor lhs, RgbColor rhs) {
        return lhs.m_code == rhs.m_code;
    }

  protected:
    // Bitmask of valid codes = 0x00RRGGBB
    static const RgbColorCode kRgbCodeMask = 0x00FFFFFF;

    static RgbColorCode validateCode(RgbColorCode code) {
        return code & kRgbCodeMask;
    }
    static bool isValidCode(RgbColorCode code) {
        return code == validateCode(code);
    }

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
class OptionalRgbColor final : public RgbColor {
  public:
    OptionalRgbColor()
            : RgbColor(kUndefinedInternalCode) {
        DEBUG_ASSERT(!isValidCode(m_code));
    }
    // Explicit conversion from RgbColor
    explicit OptionalRgbColor(RgbColor base)
            : RgbColor(base.validCode()) {
        DEBUG_ASSERT(isValidCode(m_code));
    }
    // Explicit conversion from RgbColorCode
    explicit OptionalRgbColor(RgbColorCode code)
            : RgbColor(RgbColor(validateCode(code))) {
        DEBUG_ASSERT(isValidCode(m_code) == isValidCode(code));
    }
    // Explicit conversion from an optional RgbColor that
    // is equivalent to this class. The conversion is explicit
    // to avoid ambiguities with the implicit conversion back
    // into std::optional<RgbColor>.
    explicit OptionalRgbColor(std::optional<RgbColor> optional)
            : RgbColor(optional ? optional->validCode() : kUndefinedInternalCode) {
        DEBUG_ASSERT(isValidCode(m_code) == static_cast<bool>(optional));
    }
    // Explicit conversion from QColor
    explicit OptionalRgbColor(QColor anyColor)
            : RgbColor(anyColorToInternalCode(anyColor)) {
        DEBUG_ASSERT(isValidCode(m_code) == anyColor.isValid());
    }
    OptionalRgbColor(const OptionalRgbColor&) = default;
    OptionalRgbColor(OptionalRgbColor&&) = default;

    // Explicit conversion to an optional RGB color.
    std::optional<RgbColor> optional() const {
        if (isValidCode(m_code)) {
            // Defined
            return std::make_optional(RgbColor(m_code));
        } else {
            // Undefined
            return std::nullopt;
        }
    }

    // Explicit conversion into the corresponding QColor.
    // Optionally the desired QColor that represents undefined
    // values could be provided.
    QColor toQColor(QColor undefinedColor = QColor()) const {
        if (isValidCode(m_code)) {
            // Defined
            return validQColor();
        } else {
            // Undefined
            return undefinedColor;
        }
    }

  private:
    static const RgbColorCode kUndefinedInternalCode = 0xFFFFFFFF;

    static RgbColorCode codeToInternalCode(RgbColorCode code) {
        // The unused bits of an RgbColorCode should never be set.
        DEBUG_ASSERT(isValidCode(code));
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
        return anyColorToCode(anyColor, kUndefinedInternalCode);
    }
};

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::OptionalRgbColor, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(mixxx::OptionalRgbColor)
