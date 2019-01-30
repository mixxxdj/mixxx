#include "util/color/predefinedcolorset.h"

PredefinedColorSet::PredefinedColorSet()
    : m_predefinedColorsNames(), m_defaultMap() {

    for (PredefinedColor color : allColors) {
        m_predefinedColorsNames << color.m_sName;
    }

    for (PredefinedColor color : allColors) {
        m_defaultMap.setRepresentation(color, color.m_defaultRepresentation);
    }
}
