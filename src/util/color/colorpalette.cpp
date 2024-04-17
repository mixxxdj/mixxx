#include "colorpalette.h"

mixxx::RgbColor ColorPalette::nextColor(mixxx::RgbColor color) const {
    //  Return first color if color not in palette ((-1 + 1) % size)
    return at((indexOf(color) + 1) % size());
}

mixxx::RgbColor::optional_t ColorPalette::nextColor(mixxx::RgbColor::optional_t color) const {
    if (color) {
        // If color is the last element in the palette, return no color
        if (indexOf(*color) == size() - 1) {
            return std::nullopt;
        }
        return nextColor(*color);
    }
    return at(0);
}

mixxx::RgbColor ColorPalette::previousColor(mixxx::RgbColor color) const {
    int iIndex = indexOf(color);
    if (iIndex < 0) {
        // Return last color if color not in palette
        iIndex = size() - 1;
    } else {
        iIndex = (iIndex + size() - 1) % size();
    }
    return at(iIndex);
}

mixxx::RgbColor::optional_t ColorPalette::previousColor(mixxx::RgbColor::optional_t color) const {
    if (color) {
        // If color is the first element in the palette, return no color
        if (indexOf(*color) == 0) {
            return std::nullopt;
        }
        return previousColor(*color);
    }
    return at(size() - 1);
}

mixxx::RgbColor::optional_t ColorPalette::getNthColor(
        mixxx::RgbColor::optional_t color, int steps) const {
    while (steps) {
        if (steps > 0) {
            color = nextColor(color);
            steps--;
        } else {
            color = previousColor(color);
            steps++;
        }
    }
    return color;
}

mixxx::RgbColor ColorPalette::colorForHotcueIndex(unsigned int hotcueIndex) const {
    int colorIndex;
    if (m_colorIndicesByHotcue.isEmpty()) {
        colorIndex = hotcueIndex;
    } else {
        colorIndex = m_colorIndicesByHotcue.at(hotcueIndex % m_colorIndicesByHotcue.size());
    }
    return at(colorIndex % size());
}
