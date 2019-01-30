#include "predefinedcolormap.h"

void PredefinedColorMap::setRepresentation(PredefinedColorPointer color, QColor representation) {
    m_colorNameMap[color.m_defaultRepresentation.name()] = representation.name();
}

QColor PredefinedColorMap::map(PredefinedColorPointer color) const {
    QColor defaultRepresentation = color->m_defaultRepresentation;
    if (m_colorNameMap.contains(defaultRepresentation.name())) {
        return QColor(m_colorNameMap[defaultRepresentation.name()]);
    }
    return color;
}
