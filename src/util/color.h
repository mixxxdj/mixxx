#ifndef COLOR_H
#define COLOR_H

#include "util/math.h"

#include <QColor>
#include <QMap>

#define BRIGHTNESS_TRESHOLD 130

// Map the predefined colors to another color representation of them.
//
// Since QHash has copy-on-write, making a copy of ColorsRepresentation is fast.
// A deep copy of the QHash will be made when the copy is modified.
class ColorsRepresentation final {
  public:
    // Set a color representation for a given color
    void setRepresentation(QColor color, QColor representation) {
        m_colorNameMap[color.name()] = representation.name();
    }

    // Returns the representation of a color
    QColor map(QColor color) const {
        if (m_colorNameMap.contains(color.name())) {
            return QColor(m_colorNameMap[color.name()]);
        }
        return color;
    }

  private:
    QHash<QString, QString> m_colorNameMap;
};

// These methods and properties are not thread-safe, use them only on the GUI thread
namespace Color {
    static const QColor Red      = QColor("#E6194B");
    static const QColor Green    = QColor("#3CB44B");
    static const QColor Yellow   = QColor("#FFE119");
    static const QColor Blue     = QColor("#4363D8");
    static const QColor Cyan     = QColor("#42D4F4");
    static const QColor Magenta  = QColor("#F032E6");
    static const QColor Pink     = QColor("#FABEBE");
    static const QColor Teal     = QColor("#469990");
    static const QColor Grey     = QColor("#A9A9A9");

    // Return a list with the predefined colors.
    static const QList<QColor> predefinedColors {
        Red,
        Green,
        Yellow,
        Blue,
        Cyan,
        Magenta,
        Pink,
        Teal,
        Grey,
    };

    // Return a list with the internal names of the predefined colors.
    static const QList<QLatin1String> predefinedColorsNames {
        QLatin1String("Red"),
        QLatin1String("Green"),
        QLatin1String("Yellow"),
        QLatin1String("Blue"),
        QLatin1String("Cyan"),
        QLatin1String("Magenta"),
        QLatin1String("Pink"),
        QLatin1String("Teal"),
        QLatin1String("Grey"),
    };

    // Return a predefined color code from its internal name.
    static QColor predefinedColorFromName(QLatin1String name) {
        if (name == QLatin1String("Red")) {
            return Red;
        } else if (name == QLatin1String("Green")) {
            return Green;
        } else if (name == QLatin1String("Yellow")) {
            return Yellow;
        } else if (name == QLatin1String("Blue")) {
            return Blue;
        } else if (name == QLatin1String("Cyan")) {
            return Cyan;
        } else if (name == QLatin1String("Magenta")) {
            return Magenta;
        } else if (name == QLatin1String("Pink")) {
            return Pink;
        } else if (name == QLatin1String("Teal")) {
            return Teal;
        } else if (name == QLatin1String("Grey")) {
            return Grey;
        }
        return Red;
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
        } else if (color == Cyan) {
            return QObject::tr("Cyan");
        } else if (color == Magenta) {
            return QObject::tr("Magenta");
        } else if (color == Pink) {
            return QObject::tr("Pink");
        } else if (color == Teal) {
            return QObject::tr("Teal");

        } else if (color == Grey) {
            return QObject::tr("Grey");
        }
        return QObject::tr("Undefined Color");
    };

    // The default colors representation, i.e. maps each default color to itself.
    // Stores the color's name() property, e.g. "#A9A9A9"
    //
    // It's fast to copy the default representation. See comment on ColorsRepresentation.
    static const ColorsRepresentation defaultRepresentation = [](){
        ColorsRepresentation representation = ColorsRepresentation();
        for (QColor color : predefinedColors) {
            representation.setRepresentation(color, color);
        }
        return representation;
    }();


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
