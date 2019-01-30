#ifndef PREDEFINEDCOLORMAP_H
#define PREDEFINEDCOLORMAP_H

#include <QColor>
#include <QHash>

#include "util/color/predefinedcolor.h"

// Maps a PredefinedColorMap to an actual QColor (a.k.a its representation).
//
// Initially no color has a representation set.
// Call setRepresentation(PredefinedColorPointer, QColor) to add representations and customize the color map.
//
// Uses the color's name() property as key, e.g. "#A9A9A9"
// Since QHash has copy-on-write, making a copy of ColorsRepresentation is fast.
// A deep copy of the QHash will be made when a copy is modified.
class PredefinedColorMap final {
  public:
    // Set a color representation for a given color
    void setRepresentation(PredefinedColorPointer color, QColor representation) {
        m_colorNameMap[color->m_defaultRepresentation.name()] = representation.name();
    }

    // Returnsthe representation of a color.
    // If no representation is set for color, returns color->m_defaultRepresentation.
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
