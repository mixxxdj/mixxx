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

inline bool isDimColorCustom(const QColor& color, int threshold) {
    if (threshold < 0 || threshold > 255) {
        return isDimColor(color);
    }
    return brightness(color) <= threshold;
}

// If the colorToChooseBy is darker than the global threshold,
// dimmColor will be returned. Otherwise brightColor will be returned.
inline QColor chooseColorByBrightness(QColor colorToChooseBy,
        QColor dimColor,
        QColor brightColor,
        int dimBrightThreshold) {
    return isDimColorCustom(colorToChooseBy, dimBrightThreshold)
            ? dimColor
            : brightColor;
}

// If the baseColor is darker than the global threshold,
// returns a lighter color, otherwise returns a darker color.
QColor chooseContrastColor(QColor baseColor, int dimBrightThreshold);

} // namespace Color
