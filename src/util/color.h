#ifndef COLOR_H
#define COLOR_H

#include "util/math.h"

#include <QColor>

#define BRIGHTNESS_TRESHOLD 130

// algorithm by http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
// NOTE(Swiftb0y): please suggest if I should you use other methods
// (like the W3C algorithm) or if this approach is to to performance hungry
// NOTE: the author did not take alpha transparency into account!
inline int brightness(int red, int green, int blue) {
    return static_cast<int>(sqrtf(
        red * red * .241 +
        green * green * .691 +
        blue * blue * .068)
    );
};

inline int brightness(QColor color) {
    return brightness(color.red(), color.green(), color.red());
}

inline bool isDimmColor(QColor color) {
    qDebug() << color.name();
    return brightness(color) < BRIGHTNESS_TRESHOLD;
}

// if the ColorToChooseBy is darker than the global threshold,
// the Color from the second argument will be returned.

inline QColor chooseColorByBrightnessB(bool precalculated, QColor dimmColor , QColor brightColor) {
    return precalculated ? dimmColor : brightColor;
}

inline QColor chooseColorByBrightness(QColor ColorToChooseBy, QColor dimmColor , QColor brightColor) {
    return chooseColorByBrightnessB(isDimmColor(ColorToChooseBy), dimmColor,  brightColor);
}

inline QColor chooseContrastColor(QColor colorToChooseBy) {
    return chooseColorByBrightness(colorToChooseBy, QColor(255,255,255,255), QColor(0,0,0,255));
}

inline QColor chooseContrastColorB(bool precalculated) {
    return chooseColorByBrightnessB(precalculated, QColor(255,255,255,255), QColor(0,0,0,255));
}



#endif /* COLOR_H */
