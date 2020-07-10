#pragma once

#include <QColor>

namespace Color {

int brightness(int red, int green, int blue);

inline int brightness(const QColor& color) {
    return brightness(color.red(), color.green(), color.blue());
}

inline bool isDimColor(const QColor& color) {
    return brightness(color) <= 127;
}

// If the colorToChooseBy is darker than the global threshold,
// dimmColor will be returned. Otherwise brightColor will be returned.
inline QColor chooseColorByBrightness(QColor colorToChooseBy, QColor dimColor, QColor brightColor) {
    return isDimColor(colorToChooseBy) ? dimColor : brightColor;
}

// If the baseColor is darker than the global threshold,
// returns a lighter color, otherwise returns a darker color.
QColor chooseContrastColor(QColor baseColor);

} // namespace Color
