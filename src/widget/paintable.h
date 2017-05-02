
#ifndef PAINTABLE_H
#define PAINTABLE_H

#include <QPixmap>
#include <QHash>
#include <QSharedPointer>
#include <QSvgRenderer>
#include <QImage>
#include <QScopedPointer>
#include <QPainter>
#include <QRectF>
#include <QString>

#include "skin/imgsource.h"
#include "skin/pixmapsource.h"

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

    // Takes ownership of QImage.
    Paintable(QImage* pImage, DrawMode mode);
    Paintable(const PixmapSource& source, DrawMode mode);

    QSize size() const;
    int width() const;
    int height() const;
    QRectF rect() const;
    DrawMode drawMode() const {
        return m_drawMode;
    }

    void draw(int x, int y, QPainter* pPainter);
    void draw(const QPointF& point, QPainter* pPainter,
              const QRectF& sourceRect);
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
};

#endif // PAINTABLE
