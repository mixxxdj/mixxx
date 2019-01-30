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
    void setRepresentation(PredefinedColorPointer color, QColor representation);

    // Returns the representation of a color
    QColor map(PredefinedColorPointer color) const;


  private:
    QHash<QString, QString> m_colorNameMap;
};

#endif /* PREDEFINEDCOLORMAP_H */
