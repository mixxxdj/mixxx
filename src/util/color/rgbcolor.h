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
// an optional state.
// Apart from the assignment operator this type is immutable.
class RgbColor {
  public:
    RgbColor()
            : m_internalCode(kInvalidInternalCode) {
        DEBUG_ASSERT(!*this);
    }
    /*non-explicit*/ RgbColor(RgbColorCode code)
            : m_internalCode(codeToInternalCode(code)) {
        DEBUG_ASSERT(*this);
    }
    /*non-explicit*/ RgbColor(std::optional<RgbColorCode> optionalCode)
            : m_internalCode(optionalCodeToInternalCode(optionalCode)) {
        DEBUG_ASSERT(*this == static_cast<bool>(optionalCode));
    }
    /*non-explicit*/ RgbColor(QColor anyColor)
            : m_internalCode(anyColorToInternalCode(anyColor)) {
        DEBUG_ASSERT(*this == anyColor.isValid());
    }

    // Checks if the color is valid or represents "no color",
    // i.e. is missing or undefined.
    constexpr operator bool() const noexcept {
        return m_internalCode == kInvalidInternalCode;
    }

    friend bool operator==(const RgbColor& lhs, const RgbColor& rhs) {
        return lhs.m_internalCode == rhs.m_internalCode;
    }

    // Returns the corresponding, optional RGB color code.
    std::optional<RgbColorCode> optionalCode() const {
        return internalCodeToOptionalCode(m_internalCode);
    }

    // Implicit conversion into the corresponding QColor.
    operator QColor() const {
        return internalCodeToColor(m_internalCode);
    }

  private:
    static const RgbColorCode kRgbCodeMask = 0x00FFFFFF;
    static const RgbColorCode kAlphaCodeMask = 0xFF000000;
    static const RgbColorCode kInvalidInternalCode = 0xFFFFFFFF;

    static RgbColorCode codeToInternalCode(RgbColorCode code) {
        return code & kRgbCodeMask;
    }
    static RgbColorCode optionalCodeToInternalCode(std::optional<RgbColorCode> optionalCode) {
        if (optionalCode) {
            return codeToInternalCode(*optionalCode);
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
            DEBUG_ASSERT(*this);
            return std::make_optional(m_internalCode);
        } else {
            DEBUG_ASSERT(!*this);
            return std::nullopt;
        }
    }

    QColor internalCodeToColor(RgbColorCode internalCode) const {
        if (internalCode == (internalCode & kRgbCodeMask)) {
            DEBUG_ASSERT(*this);
            return QRgb(m_internalCode | kAlphaCodeMask);
        } else {
            DEBUG_ASSERT(!*this);
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
