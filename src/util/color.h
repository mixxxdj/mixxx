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
    static const QString Red      = "#E6194B";
    static const QString Green    = "#3CB44B";
    static const QString Yellow   = "#FFE119";
    static const QString Blue     = "#4363D8";
    static const QString Orange   = "#F58231";
    static const QString Purple   = "#911EB4";
    static const QString Cyan     = "#42D4F4";
    static const QString Magenta  = "#F032E6";
    static const QString Lime     = "#BFEF45";
    static const QString Pink     = "#FABEBE";
    static const QString Teal     = "#469990";
    static const QString Lavender = "#E6BEFF";
    static const QString Brown    = "#9A6324";
    static const QString Beige    = "#FFFAC8";
    static const QString Maroon   = "#800000";
    static const QString Mint     = "#AAFFC3";
    static const QString Olive    = "#808000";
    static const QString Apricot  = "#FFD8B1";
    static const QString Navy     = "#000075";
    static const QString Grey     = "#A9A9A9";
    static const QString White    = "#FFFFFF";
    static const QString Black    = "#000000";

    // Return a list with the predefined colors.
    static QList<QColor> predefinedColors() {
        return QList<QColor> {
            QColor(Red),
            QColor(Green),
            QColor(Yellow),
            QColor(Blue),
            QColor(Orange),
            QColor(Purple),
            QColor(Cyan),
            QColor(Magenta),
            QColor(Lime),
            QColor(Pink),
            QColor(Teal),
            QColor(Lavender),
            QColor(Brown),
            QColor(Beige),
            QColor(Maroon),
            QColor(Mint),
            QColor(Olive),
            QColor(Apricot),
            QColor(Navy),
            QColor(Grey),
            QColor(White),
            QColor(Black),
        };
    };

    // Return a list with the internal names of the predefined colors.
    static QList<QString> predefinedColorsNames() {
        return QList<QString> {
            "Red",
            "Green",
            "Yellow",
            "Blue",
            "Orange",
            "Purple",
            "Cyan",
            "Magenta",
            "Lime",
            "Pink",
            "Teal",
            "Lavender",
            "Brown",
            "Beige",
            "Maroon",
            "Mint",
            "Olive",
            "Apricot",
            "Navy",
            "Grey",
            "White",
            "Black",
        };
    };

    // Return a predefined color code from its internal name.
    static QString predefinedColorFromName(QString name) {
        if (name == "Red") {
            return Red;
        } else if (name == "Green") {
            return Green;
        } else if (name == "Yellow") {
            return Yellow;
        } else if (name == "Blue") {
            return Blue;
        } else if (name == "Orange") {
            return Orange;
        } else if (name == "Purple") {
            return Purple;
        } else if (name == "Cyan") {
            return Cyan;
        } else if (name == "Magenta") {
            return Magenta;
        } else if (name == "Lime") {
            return Lime;
        } else if (name == "Pink") {
            return Pink;
        } else if (name == "Teal") {
            return Teal;
        } else if (name == "Lavender") {
            return Lavender;
        } else if (name == "Brown") {
            return Brown;
        } else if (name == "Beige") {
            return Beige;
        } else if (name == "Maroon") {
            return Maroon;
        } else if (name == "Mint") {
            return Mint;
        } else if (name == "Olive") {
            return Olive;
        } else if (name == "Apricot") {
            return Apricot;
        } else if (name == "Navy") {
            return Navy;
        } else if (name == "Grey") {
            return Grey;
        } else if (name == "White") {
            return White;
        } else if (name == "Black") {
            return Black;
        }
        return Black;
    };

    // Return the localized name of a predefined color.
    // Returns "Undefined Color" if color is not a predefined color.
    static QString displayName(QColor color) {
        if (color.name().toUpper() == Red.toUpper()) {
            return QObject::tr("Red");
        } else if (color.name().toUpper() == Green.toUpper()) {
            return QObject::tr("Green");
        } else if (color.name().toUpper() == Yellow.toUpper()) {
            return QObject::tr("Yellow");
        } else if (color.name().toUpper() == Blue.toUpper()) {
            return QObject::tr("Blue");
        } else if (color.name().toUpper() == Orange.toUpper()) {
            return QObject::tr("Orange");
        } else if (color.name().toUpper() == Purple.toUpper()) {
            return QObject::tr("Purple");
        } else if (color.name().toUpper() == Cyan.toUpper()) {
            return QObject::tr("Cyan");
        } else if (color.name().toUpper() == Magenta.toUpper()) {
            return QObject::tr("Magenta");
        } else if (color.name().toUpper() == Lime.toUpper()) {
            return QObject::tr("Lime");
        } else if (color.name().toUpper() == Pink.toUpper()) {
            return QObject::tr("Pink");
        } else if (color.name().toUpper() == Teal.toUpper()) {
            return QObject::tr("Teal");
        } else if (color.name().toUpper() == Lavender.toUpper()) {
            return QObject::tr("Lavender");
        } else if (color.name().toUpper() == Brown.toUpper()) {
            return QObject::tr("Brown");
        } else if (color.name().toUpper() == Beige.toUpper()) {
            return QObject::tr("Beige");
        } else if (color.name().toUpper() == Maroon.toUpper()) {
            return QObject::tr("Maroon");
        } else if (color.name().toUpper() == Mint.toUpper()) {
            return QObject::tr("Mint");
        } else if (color.name().toUpper() == Olive.toUpper()) {
            return QObject::tr("Olive");
        } else if (color.name().toUpper() == Apricot.toUpper()) {
            return QObject::tr("Apricot");
        } else if (color.name().toUpper() == Navy.toUpper()) {
            return QObject::tr("Navy");
        } else if (color.name().toUpper() == Grey.toUpper()) {
            return QObject::tr("Grey");
        } else if (color.name().toUpper() == White.toUpper()) {
            return QObject::tr("White");
        } else if (color.name().toUpper() == Black.toUpper()) {
            return QObject::tr("Black");
        }
        return QObject::tr("Undefined Color");
    };

    // Returns a new default colors representation, i.e. maps each default color to itself.
    // Stores the color's name() property, e.g. "#A9A9A9"
    static std::unique_ptr<ColorsRepresentation> defaultRepresentation() {
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
