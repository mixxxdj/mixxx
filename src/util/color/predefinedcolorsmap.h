#ifndef PREDEFINEDCOLORSMAP_H
#define PREDEFINEDCOLORSMAP_H

#include <QColor>
#include <QHash>

#include "util/color/predefinedcolor.h"

// Maps a PredefinedColor to custom Rgba color.
//
// Initially no color has a custom Rgba set.
// Call setCustomRgba(PredefinedColorPointer, QColor) to add a custom Rgba for a predefined color
// and customize the color map.
//
// This class uses the color's name() property as key, e.g. "#A9A9A9"
// Since QHash has copy-on-write, making a copy of PredefinedColorsMap is fast.
// A deep copy of the QHash will be made when a copy is modified.
class PredefinedColorsMap final {
  public:
    // Set a custom Rgba for a given color
    void setCustomRgba(PredefinedColorPointer color, QColor cutomizedRgba) {
        m_colorNameMap[color->m_defaultRgba.name()] = cutomizedRgba.name();
    }

    // Returns the custom Rgba of a color.
    // If no custom Rgba is set for color, returns color->m_defaultRgba.
    QColor map(PredefinedColorPointer color) const {
        QColor defaultRgba = color->m_defaultRgba;
        if (m_colorNameMap.contains(defaultRgba.name())) {
            return QColor(m_colorNameMap[defaultRgba.name()]);
        }
        return defaultRgba;
    }


  private:
    QHash<QString, QString> m_colorNameMap;
};

#endif /* PREDEFINEDCOLORSMAP_H */
