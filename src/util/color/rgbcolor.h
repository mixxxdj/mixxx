#pragma once

#include <QColor>
#include <QVariant>
#include <QtDebug>
#include <QtGlobal>

#include "util/assert.h"
#include "util/optional.h"

namespace mixxx {

// Type-safe wrapper for 24-bit RGB color codes without an alpha
// channel. Code values are implicitly validated upon construction.
//
// Apart from the assignment operator this type is immutable.
class RgbColor {
  public:
    // A pure 24-bit 0xxRRGGBB color code with 8-bit per channel
    // without an an alpha channel.
    // We are using a separate typedef, because QRgb implicitly
    // includes an alpha channel whereas this type does not!
    typedef quint32 code_t;

    static constexpr code_t validateCode(code_t code) {
        return code & kRgbCodeMask;
    }
    static bool isValidCode(code_t code) {
        return code == validateCode(code);
    }

    // The default constructor is not available, because there is
    // no common default value that fits all possible use cases!
    RgbColor() = delete;
    // Explicit conversion from code_t with implicit validation.
    explicit constexpr RgbColor(code_t code)
            : m_code(validateCode(code)) {
    }
    // Explicit conversion from QColor.
    RgbColor(QColor anyColor, code_t codeIfInvalid)
            : m_code(anyColorToCode(anyColor, codeIfInvalid)) {
    }

    // Implicit conversion to a color code.
    constexpr operator code_t() const {
        return m_code;
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
    static constexpr optional_t optional(code_t code) {
        return optional(RgbColor(code));
    }
    static constexpr optional_t optional(RgbColor color) {
        return std::make_optional(color);
    }
    static optional_t optional(QColor color) {
        if (color.isValid()) {
            return optional(validateCode(color.rgb()));
        } else {
            return nullopt();
        }
    }
    static optional_t optional(const QVariant& varCode) {
        if (varCode.isNull()) {
            return nullopt();
        } else {
            DEBUG_ASSERT(varCode.canConvert(QMetaType::UInt));
            bool ok = false;
            const auto code = varCode.toUInt(&ok);
            VERIFY_OR_DEBUG_ASSERT(ok) {
                return nullopt();
            }
            return optional(static_cast<code_t>(code));
        }
    }

  protected:
    // Bitmask of valid codes = 0x00RRGGBB
    static constexpr code_t kRgbCodeMask = 0x00FFFFFF;

    static code_t anyColorToCode(QColor anyColor, code_t codeIfInvalid) {
        if (anyColor.isValid()) {
            // Strip alpha channel bits!
            return validateCode(anyColor.rgb());
        } else {
            return codeIfInvalid;
        }
    }

    code_t m_code;
};

inline bool operator!=(RgbColor lhs, RgbColor rhs) {
    return !(lhs == rhs);
}

// Explicit conversion of both non-optional and optional
// RgbColor values to QColor as overloaded free functions.

inline QColor toQColor(RgbColor color) {
    return QColor::fromRgb(color);
}

inline QColor toQColor(
        RgbColor::optional_t optional,
        QColor defaultColor = QColor()) {
    if (optional) {
        return toQColor(*optional);
    } else {
        return defaultColor;
    }
}

// Explicit conversion of both non-optional and optional
// RgbColor values to QVariant as overloaded free functions.

inline QVariant toQVariant(RgbColor color) {
    return QVariant(static_cast<RgbColor::code_t>(color));
}

inline QVariant toQVariant(RgbColor::optional_t optional) {
    if (optional) {
        return toQVariant(*optional);
    } else {
        return QVariant();
    }
}

// Explicit conversion of both non-optional and optional
// RgbColor values to QString with the format #RRGGBB,
// e.g. for tool.

inline QString toQString(
        mixxx::RgbColor color) {
    return mixxx::toQColor(color).name();
}

// String representation #RRGGBB, e.g. for tool.
inline QString toQString(
        mixxx::RgbColor::optional_t color,
        const QString& defaultString = QString()) {
    if (color) {
        return toQString(*color);
    } else {
        return defaultString;
    }
}

// Debug output stream operators

inline QDebug operator<<(QDebug dbg, RgbColor color) {
    return dbg << toQColor(color);
}

inline QDebug operator<<(QDebug dbg, RgbColor::optional_t optional) {
    return dbg << toQColor(optional);
}

} // namespace mixxx

// Assumption: A primitive type wrapped into std::optional is
// still a primitive type.
Q_DECLARE_TYPEINFO(std::optional<mixxx::RgbColor>, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(std::optional<mixxx::RgbColor>)
