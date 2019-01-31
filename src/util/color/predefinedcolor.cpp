#include "util/color/predefinedcolor.h"

#include "util/color/color.h"

PredefinedColor::PredefinedColor(QColor defaultRepresentation, QString sName, QString sDisplayName, int iId)
    : m_defaultRepresentation(defaultRepresentation),
      m_sName(sName),
      m_sDisplayName(sDisplayName),
      m_iId(iId) {
}
