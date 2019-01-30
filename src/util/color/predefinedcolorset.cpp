#include "util/color/predefinedcolorset.h"

PredefinedColorSet::PredefinedColorSet()
    : m_predefinedColorsNames(), m_defaultMap() {

    for (PredefinedColorPointer color : allColors) {
        m_predefinedColorsNames << color->m_sName;
        m_defaultMap.setRepresentation(color, color->m_defaultRepresentation);
    }
}
