#ifndef COLOR_H
#define COLOR_H

#include "util/math.h"
#include "util/memory.h"

#include <QColor>
#include <QMap>

#define BRIGHTNESS_TRESHOLD 130

// Map the predefined colors to another color representation of them.
class ColorsRepresentation final {
  public:
    // Set a color representation for a given color
    void setRepresentation(QColor color, QColor representation) {
        colorNameMap[color.name()] = representation.name();
    }

    // Returns the representation of a color
    QColor map(QColor color) const {
        if (colorNameMap.contains(color.name())) {
            return QColor(colorNameMap[color.name()]);
        }
        return color;
    }

  private:
    QHash<QString, QString> colorNameMap;
};

namespace Color {
    static const QColor Red      = QColor("#E6194B");
    static const QColor Green    = QColor("#3CB44B");
    static const QColor Yellow   = QColor("#FFE119");
    static const QColor Blue     = QColor("#4363D8");
    static const QColor Orange   = QColor("#F58231");
    static const QColor Purple   = QColor("#911EB4");
    static const QColor Cyan     = QColor("#42D4F4");
    static const QColor Magenta  = QColor("#F032E6");
    static const QColor Lime     = QColor("#BFEF45");
    static const QColor Pink     = QColor("#FABEBE");
    static const QColor Teal     = QColor("#469990");
    static const QColor Lavender = QColor("#E6BEFF");
    static const QColor Brown    = QColor("#9A6324");
    static const QColor Beige    = QColor("#FFFAC8");
    static const QColor Maroon   = QColor("#800000");
    static const QColor Mint     = QColor("#AAFFC3");
    static const QColor Olive    = QColor("#808000");
    static const QColor Apricot  = QColor("#FFD8B1");
    static const QColor Navy     = QColor("#000075");
    static const QColor Grey     = QColor("#A9A9A9");
    static const QColor White    = QColor("#FFFFFF");
    static const QColor Black    = QColor("#000000");

    // Return a list with the predefined colors.
    static QList<QColor> predefinedColors() {
        return QList<QColor> {
            Red,
            Green,
            Yellow,
            Blue,
            Orange,
            Purple,
            Cyan,
            Magenta,
            Lime,
            Pink,
            Teal,
            Lavender,
            Brown,
            Beige,
            Maroon,
            Mint,
            Olive,
            Apricot,
            Navy,
            Grey,
            White,
            Black,
        };
    };

    // Return a list with the internal names of the predefined colors.
    static QList<QLatin1String> predefinedColorsNames() {
        return QList<QLatin1String> {
            QLatin1String("Red"),
            QLatin1String("Green"),
            QLatin1String("Yellow"),
            QLatin1String("Blue"),
            QLatin1String("Orange"),
            QLatin1String("Purple"),
            QLatin1String("Cyan"),
            QLatin1String("Magenta"),
            QLatin1String("Lime"),
            QLatin1String("Pink"),
            QLatin1String("Teal"),
            QLatin1String("Lavender"),
            QLatin1String("Brown"),
            QLatin1String("Beige"),
            QLatin1String("Maroon"),
            QLatin1String("Mint"),
            QLatin1String("Olive"),
            QLatin1String("Apricot"),
            QLatin1String("Navy"),
            QLatin1String("Grey"),
            QLatin1String("White"),
            QLatin1String("Black"),
        };
    };

    // Return a predefined color code from its internal name.
    // TODO: use literals here
    static QColor predefinedColorFromName(QLatin1String name) {
        if (name == QLatin1String("Red")) {
            return Red;
        } else if (name == QLatin1String("Green")) {
            return Green;
        } else if (name == QLatin1String("Yellow")) {
            return Yellow;
        } else if (name == QLatin1String("Blue")) {
            return Blue;
        } else if (name == QLatin1String("Orange")) {
            return Orange;
        } else if (name == QLatin1String("Purple")) {
            return Purple;
        } else if (name == QLatin1String("Cyan")) {
            return Cyan;
        } else if (name == QLatin1String("Magenta")) {
            return Magenta;
        } else if (name == QLatin1String("Lime")) {
            return Lime;
        } else if (name == QLatin1String("Pink")) {
            return Pink;
        } else if (name == QLatin1String("Teal")) {
            return Teal;
        } else if (name == QLatin1String("Lavender")) {
            return Lavender;
        } else if (name == QLatin1String("Brown")) {
            return Brown;
        } else if (name == QLatin1String("Beige")) {
            return Beige;
        } else if (name == QLatin1String("Maroon")) {
            return Maroon;
        } else if (name == QLatin1String("Mint")) {
            return Mint;
        } else if (name == QLatin1String("Olive")) {
            return Olive;
        } else if (name == QLatin1String("Apricot")) {
            return Apricot;
        } else if (name == QLatin1String("Navy")) {
            return Navy;
        } else if (name == QLatin1String("Grey")) {
            return Grey;
        } else if (name == QLatin1String("White")) {
            return White;
        } else if (name == QLatin1String("Black")) {
            return Black;
        }
        return Black;
    };

    // Return the localized name of a predefined color.
    // Returns "Undefined Color" if color is not a predefined color.
    static QString displayName(QColor color) {
        if (color == Red) {
            return QObject::tr("Red");
        } else if (color == Green) {
            return QObject::tr("Green");
        } else if (color == Yellow) {
            return QObject::tr("Yellow");
        } else if (color == Blue) {
            return QObject::tr("Blue");
        } else if (color == Orange) {
            return QObject::tr("Orange");
        } else if (color == Purple) {
            return QObject::tr("Purple");
        } else if (color == Cyan) {
            return QObject::tr("Cyan");
        } else if (color == Magenta) {
            return QObject::tr("Magenta");
        } else if (color == Lime) {
            return QObject::tr("Lime");
        } else if (color == Pink) {
            return QObject::tr("Pink");
        } else if (color == Teal) {
            return QObject::tr("Teal");
        } else if (color == Lavender) {
            return QObject::tr("Lavender");
        } else if (color == Brown) {
            return QObject::tr("Brown");
        } else if (color == Beige) {
            return QObject::tr("Beige");
        } else if (color == Maroon) {
            return QObject::tr("Maroon");
        } else if (color == Mint) {
            return QObject::tr("Mint");
        } else if (color == Olive) {
            return QObject::tr("Olive");
        } else if (color == Apricot) {
            return QObject::tr("Apricot");
        } else if (color == Navy){
            return QObject::tr("Navy");
        } else if (color == Grey) {
            return QObject::tr("Grey");
        } else if (color == White) {
            return QObject::tr("White");
        } else if (color == Black) {
            return QObject::tr("Black");
        }
        return QObject::tr("Undefined Color");
    };

    // Returns a new default colors representation, i.e. maps each default color to itself.
    // Stores the color's name() property, e.g. "#A9A9A9"
    static std::unique_ptr<ColorsRepresentation> makeDefaultRepresentation() {
        std::unique_ptr<ColorsRepresentation> representation = std::make_unique<ColorsRepresentation>();
        for (QColor color : predefinedColors()) {
            representation->setRepresentation(color, color);
        }
        return representation;
    }


    // algorithm by http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
    // NOTE(Swiftb0y): please suggest if I should you use other methods
    // (like the W3C algorithm) or if this approach is to to performance hungry
    // NOTE: the author did not take alpha transparency into account!
    static inline int brightness(int red, int green, int blue) {
        return static_cast<int>(sqrtf(
            red * red * .241 +
            green * green * .691 +
            blue * blue * .068)
        );
    };

    static inline int brightness(QColor color) {
        return brightness(color.red(), color.green(), color.red());
    }

    static inline bool isDimmColor(QColor color) {
//        qDebug() << color.name();
        return brightness(color) < BRIGHTNESS_TRESHOLD;
    }

    // if the ColorToChooseBy is darker than the global threshold,
    // the Color from the second argument will be returned.

    static inline QColor chooseColorByBrightnessB(bool precalculated, QColor dimmColor , QColor brightColor) {
        return precalculated ? dimmColor : brightColor;
    }

    static inline QColor chooseColorByBrightness(QColor ColorToChooseBy, QColor dimmColor , QColor brightColor) {
        return chooseColorByBrightnessB(isDimmColor(ColorToChooseBy), dimmColor,  brightColor);
    }

    static inline QColor chooseContrastColor(QColor colorToChooseBy) {
        return chooseColorByBrightness(colorToChooseBy, QColor(255,255,255,255), QColor(0,0,0,255));
    }

    static inline QColor chooseContrastColorB(bool precalculated) {
        return chooseColorByBrightnessB(precalculated, QColor(255,255,255,255), QColor(0,0,0,255));
    }

};
#endif /* COLOR_H */
