#pragma once

#include <QColor>
#include <QJSValue>
#include <QMap>

#include "util/assert.h"

/// ColorMapper allows to find the nearest color representation of a given color
/// in a set of fixed colors. Additional user data (e.g. MIDI byte values) can
/// be linked to colors in the color set as QVariant.
class ColorMapper final {
  public:
    ColorMapper() = delete;
    explicit ColorMapper(const QMap<QRgb, QVariant>& availableColors)
            : m_availableColors(availableColors) {
        DEBUG_ASSERT(!m_availableColors.isEmpty());
    }

    QRgb getNearestColor(QRgb desiredColor);
    QVariant getValueForNearestColor(QRgb desiredColor);

  private:
    const QMap<QRgb, QVariant> m_availableColors;
    QMap<QRgb, QRgb> m_cache;
};
