#include "util/color/predefinedcolor.h"

#include "util/color/color.h"

PredefinedColor::PredefinedColor(QColor defaultRgba, QString sName, QString sDisplayName, int iId)
    : m_defaultRgba(defaultRgba),
      m_sName(sName),
      m_sDisplayName(sDisplayName),
      m_iId(iId) {
}
