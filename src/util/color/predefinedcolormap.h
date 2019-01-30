#ifndef PREDEFINEDCOLORMAP_H
#define PREDEFINEDCOLORMAP_H

#include <QColor>
#include <QHash>

#include "util/color/predefinedcolor.h"

// Map the predefined colors to another color representation of them.
//
// Uses the color's name() property as key, e.g. "#A9A9A9"
// Since QHash has copy-on-write, making a copy of ColorsRepresentation is fast.
// A deep copy of the QHash will be made when the copy is modified.
class PredefinedColorMap final {
  public:
    // Set a color representation for a given color
    void setRepresentation(PredefinedColorPointer color, QColor representation) {
        m_colorNameMap[color->m_defaultRepresentation.name()] = representation.name();
    }

    // Returns the representation of a color
    QColor map(PredefinedColorPointer color) const {
        QColor defaultRepresentation = color->m_defaultRepresentation;
        if (m_colorNameMap.contains(defaultRepresentation.name())) {
            return QColor(m_colorNameMap[defaultRepresentation.name()]);
        }
        return defaultRepresentation;
    }


  private:
    QHash<QString, QString> m_colorNameMap;
};

#endif /* PREDEFINEDCOLORMAP_H */
