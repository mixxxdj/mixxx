#pragma once

#include "util/color/predefinedcolorsset.h"

namespace Color {

extern const PredefinedColorsSet kPredefinedColorsSet;

int brightness(int red, int green, int blue);

inline int brightness(const QColor& color) {
    return brightness(color.red(), color.green(), color.red());
}

inline bool isDimmColor(const QColor& color) {
    return brightness(color) <= 127;
}

// If the colorToChooseBy is darker than the global threshold,
// dimmColor will be returned. Otherwise brightColor will be returned.
inline QColor chooseColorByBrightness(QColor colorToChooseBy, QColor dimmColor, QColor brightColor) {
    return isDimmColor(colorToChooseBy) ? dimmColor : brightColor;
}

// If the baseColor is darker than the global threshold,
// returns a lighter color, otherwise returns a darker color.
QColor chooseContrastColor(QColor baseColor);

} // namespace Color
