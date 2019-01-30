#ifndef PREDEFINEDCOLORSET_H
#define PREDEFINEDCOLORSET_H

#include <QColor>
#include <QHash>

#include "util/color/predefinedcolorset.h"
#include "util/math.h"

namespace Color {
    static const PredefinedColorSet predefinedColorSet = PredefinedColorSet();

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

    static inline int brightness(const QColor& color) {
        return brightness(color.red(), color.green(), color.red());
    }

    static inline bool isDimmColor(const QColor& color) {
        return brightness(color) <= 127;
    }

    // If the colorToChooseBy is darker than the global threshold,
    // dimmColor will be returned. Otherwise brightColor will be returned.
    static inline QColor chooseColorByBrightness(QColor colorToChooseBy, QColor dimmColor , QColor brightColor) {
        return isDimmColor(colorToChooseBy) ? dimmColor : brightColor;
    }

    // If the baseColor is darker than the global threshold,
    // returns white, otherwise returns black.
    static inline QColor chooseContrastColor(QColor baseColor) {
        QColor lightColor = baseColor.lighter();
        QColor darkColor = baseColor.darker();

        // QColor::lighter() multiplies the HSV Value by some factor. When baseColor is black, Value is 0,
        // thus after multiplication it's still 0 and we get the same color.
        // We manually set lightColor to darkGray in this case.
        if (baseColor.toHsv().value() == 0) {
            lightColor = Qt::darkGray;
            lightColor.setAlpha(baseColor.alpha());
        }
        return chooseColorByBrightness(baseColor, lightColor, darkColor);
    }
}
#endif /* PREDEFINEDCOLORSET_H */
