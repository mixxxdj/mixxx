#pragma once

#include <QImage>
#include <QRectF>
#include <QString>
#include <memory>

#include "skin/legacy/imgsource.h"
#include "skin/legacy/pixmapsource.h"

class QPainter;
class QPixmap;
class QSvgRenderer;

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
    QImage toImage() const;
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
    void mayCorrectColors();

    std::unique_ptr<QPixmap> m_pPixmap;
    std::unique_ptr<QSvgRenderer> m_pSvg;
    DrawMode m_drawMode;
    QRectF m_lastSourceRect;
};
