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
    typedef QRgb code_t;

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

    ///////////////////////////////////////////////////////////////////
    // Conversion functions from/to Qt types
    ///////////////////////////////////////////////////////////////////

    // Explicit conversion of both non-optional and optional
    // RgbColor values to/from QColor as overloaded free functions.

    static QColor toQColor(RgbColor color) {
        return QColor::fromRgb(color);
    }

    static QColor toQColor(
            RgbColor::optional_t optional,
            const QColor& defaultColor = QColor()) {
        if (optional) {
            return toQColor(*optional);
        } else {
            return defaultColor;
        }
    }

    static RgbColor::optional_t fromQColor(
            const QColor& color,
            RgbColor::optional_t defaultColor = RgbColor::nullopt()) {
        if (color.isValid()) {
            return RgbColor::optional(
                    RgbColor::validateCode(color.rgb()));
        } else {
            return defaultColor;
        }
    }

    ///////////////////////////////////////////////////////////////////
    // Sorting
    ///////////////////////////////////////////////////////////////////

    // Number of distinct values a 24-bit color code can take. Used as the
    // multiplier that packs a hue into the high bits of a sort key.
    static constexpr qint64 kColorCodeRange = 1LL << 24;

    // Sort keys for colors that do not have a meaningful hue. They are
    // deliberately greater than any real hue (0..359) so that achromatic
    // colors are grouped after all chromatic ones, and unset colors are
    // grouped after all colored ones.
    static constexpr int kSortKeyAchromatic = 360;
    static constexpr int kSortKeyUnset = 361;

    // Returns a sort key that orders colors by hue, i.e. colors of a similar
    // hue end up next to each other, instead of by their internal color code.
    //
    // The key is monotonically increasing with the hue and additionally
    // incorporates the color code, which yields a total order. That matters
    // because the library sorts incrementally by binary search, which requires
    // a strict weak ordering to be consistent with the initial SQL query.
    //
    // NOTE: This must stay in sync with the `mixxx_hue()` SQL function
    // registered in mixxx::DbConnection, which is used to build the
    // "ORDER BY" clause for the color column (see ColumnCache). Both sides
    // have to agree, otherwise incrementally re-sorted rows would jump around.
    static qint64 sortKey(RgbColor color) {
        const QColor qColor = toQColor(color);
        // QColor::hue() returns an undefined value (-1) for achromatic
        // colors, i.e. when saturation is 0 (grey, black, white).
        const int hue = qColor.saturation() == 0
                ? kSortKeyAchromatic
                : qColor.hue();
        return static_cast<qint64>(hue) * kColorCodeRange +
                static_cast<qint64>(color.m_code);
    }

    static qint64 sortKey(optional_t color) {
        return color
                ? sortKey(*color)
                : static_cast<qint64>(kSortKeyUnset) * kColorCodeRange;
    }

    // Explicit conversion of both non-optional and optional
    // RgbColor values to/from QString in the format #RRGGBB,
    // e.g. for tool tips or settings.

    static QString toQString(
            mixxx::RgbColor color) {
        return toQColor(color).name(QColor::HexRgb);
    }

    static QString toQString(
            mixxx::RgbColor::optional_t color,
            const QString& defaultString = QString()) {
        if (color) {
            return toQString(*color);
        } else {
            return defaultString;
        }
    }

    static RgbColor::optional_t fromQString(
            const QString& hexCode,
            RgbColor::optional_t defaultColor = RgbColor::nullopt()) {
        const auto color = QColor(hexCode);
        if (color.isValid()) {
            return fromQColor(color);
        } else {
            return defaultColor;
        }
    }

    // Explicit conversion of both non-optional and optional
    // RgbColor values to QVariant as overloaded free functions.
    //
    // The default versions encode the internal color code as
    // an unsigned integer. The `Color` and `String` variants
    // use the corresponding mapping to QColor and QString
    // respectively.

    static QVariant toQVariant(
            RgbColor color) {
        return QVariant(static_cast<code_t>(color));
    }

    static QVariant toQVariantColor(
            RgbColor color) {
        return QVariant(toQColor(color));
    }

    static QVariant toQVariantString(
            RgbColor color) {
        return QVariant(toQString(color));
    }

    static QVariant toQVariant(
            optional_t optional,
            const QVariant& defaultVariant = QVariant()) {
        if (optional) {
            return toQVariant(*optional);
        } else {
            return defaultVariant;
        }
    }

    static QVariant toQVariantColor(
            optional_t optional,
            const QVariant& defaultVariant = QVariant()) {
        if (optional) {
            return toQVariantColor(*optional);
        } else {
            return defaultVariant;
        }
    }

    static QVariant toQVariantString(
            optional_t optional,
            const QVariant& defaultVariant = QVariant()) {
        if (optional) {
            return toQVariantString(*optional);
        } else {
            return defaultVariant;
        }
    }

    static optional_t fromQVariant(
            const QVariant& varCode,
            optional_t defaultColor = nullopt()) {
        if (varCode.isNull()) {
            return defaultColor;
        }
        VERIFY_OR_DEBUG_ASSERT(varCode.canConvert<uint>()) {
            return defaultColor;
        }
        const auto value = varCode.value<code_t>();
        return RgbColor::optional(value);
    }

    static optional_t fromQVariantColor(
            const QVariant& varColor,
            optional_t defaultColor = nullopt()) {
        if (varColor.isNull()) {
            return defaultColor;
        }
        VERIFY_OR_DEBUG_ASSERT(varColor.canConvert<QColor>()) {
            return defaultColor;
        }
        const auto value = varColor.value<QColor>();
        return fromQColor(value);
    }

    static optional_t fromQVariantString(
            const QVariant& varString,
            optional_t defaultColor = nullopt()) {
        if (varString.isNull()) {
            return defaultColor;
        }
        VERIFY_OR_DEBUG_ASSERT(varString.canConvert<QString>()) {
            return defaultColor;
        }
        const auto value = varString.value<QString>();
        return fromQString(value);
    }

  protected:
    // Bitmask of valid codes = 0x00RRGGBB
    static constexpr code_t kRgbCodeMask = 0x00FFFFFF;

    code_t m_code;
};

inline bool operator!=(RgbColor lhs, RgbColor rhs) {
    return !(lhs == rhs);
}

// Debug output stream operators

inline QDebug operator<<(QDebug dbg, RgbColor color) {
    return dbg << RgbColor::toQString(color).toLatin1().constData();
}

inline QDebug operator<<(QDebug dbg, const RgbColor::optional_t& optionalColor) {
    return dbg << RgbColor::toQString(optionalColor).toLatin1().constData();
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::RgbColor, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(std::optional<mixxx::RgbColor>, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(std::optional<mixxx::RgbColor>)
