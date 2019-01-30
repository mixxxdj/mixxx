#include "util/color/predefinedcolor.h"

#include "utils/color/color.h"

PredefinedColor::PredefinedColor(QColor defaultRepresentation, QString sName, QString sDisplayName)
    : m_defaultRepresentation(defaultRepresentation),
      m_sName(sName),
      m_sDisplayName(sDisplayName),
      m_iBrightness(Color::brightness(defaultRepresentation)) {
}
