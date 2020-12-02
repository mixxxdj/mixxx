
#ifndef PAINTABLE_H
#define PAINTABLE_H

#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QSvgRenderer>

#include "skin/imgsource.h"
#include "skin/pixmapsource.h"

class QPainter;

// Wrapper around QImage and QSvgRenderer to support rendering SVG images in
// high fidelity.
class Paintable {
  public:
    enum DrawMode {
        // Draw the image in its native dimensions with no stretching or tiling.
        FIXED,
        // Stretch the image.
        STRETCH,
        // Stretch the image maintaining its aspect ratio.
        STRETCH_ASPECT,
        // Tile the image.
        TILE
    };

    Paintable(const PixmapSource& source, DrawMode mode, double scaleFactor);

    QSize size() const;
    int width() const;
    int height() const;
    QRectF rect() const;
    DrawMode drawMode() const {
        return m_drawMode;
    }

    void draw(int x, int y, QPainter* pPainter);
    void draw(const QRectF& targetRect, QPainter* pPainter);
    void draw(const QRectF& targetRect, QPainter* pPainter,
              const QRectF& sourceRect);
    void drawCentered(const QRectF& targetRect, QPainter* pPainter,
                      const QRectF& sourceRect);
    bool isNull() const;

    static DrawMode DrawModeFromString(const QString& str);
    static QString DrawModeToString(DrawMode mode);
    static QString getAltFileName(const QString& fileName);

  private:
    void drawInternal(const QRectF& targetRect, QPainter* pPainter,
                      const QRectF& sourceRect);

    QScopedPointer<QPixmap> m_pPixmap;
    QScopedPointer<QSvgRenderer> m_pSvg;
    DrawMode m_drawMode;
    PixmapSource m_source;
};

#endif // PAINTABLE
