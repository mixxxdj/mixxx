#include "colorpalette.h"

mixxx::RgbColor ColorPalette::nextColor(mixxx::RgbColor color) const {
    //  Return first color if color not in palette ((-1 + 1) % size)
    return at((indexOf(color) + 1) % size());
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

mixxx::RgbColor ColorPalette::colorForHotcueIndex(unsigned int hotcueIndex) const {
    int colorIndex;
    if (m_colorIndicesByHotcue.isEmpty()) {
        colorIndex = hotcueIndex;
    } else {
        colorIndex = m_colorIndicesByHotcue.at(hotcueIndex % m_colorIndicesByHotcue.size());
    }
    return at(colorIndex % size());
}
