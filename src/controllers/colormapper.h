#ifndef COLORMAPPER_H
#define COLORMAPPER_H

#include <QColor>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QVariant>

#include "util/assert.h"

class ColorMapper final : public QObject {
    Q_OBJECT
  public:
    ColorMapper() = delete;
    ColorMapper(const QMap<QRgb, QVariant> availableColors)
            : m_availableColors(availableColors) {
        DEBUG_ASSERT(!m_availableColors.isEmpty());
    }

    ~ColorMapper() = default;

    QPair<QRgb, QVariant> getNearestColor(QRgb desiredColor);

  private:
    const QMap<QRgb, QVariant> m_availableColors;
    QMap<QRgb, QRgb> m_cache;
};

#endif /* COLORMAPPER_H */
