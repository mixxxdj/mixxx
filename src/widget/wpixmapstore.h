/***************************************************************************
                          wpixmapstore.h  -  description
                             -------------------
    begin                : Mon Jun 28 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPIXMAPSTORE_H
#define WPIXMAPSTORE_H

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
    Paintable(const QString& fileName, DrawMode mode);
    Paintable(const PixmapSource& source, DrawMode mode);

    QSize size() const;
    int width() const;
    int height() const;
    QRectF rect() const;
    DrawMode drawMode() const {
        return m_draw_mode;
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

  private:
    void drawInternal(const QRectF& targetRect, QPainter* pPainter,
                      const QRectF& sourceRect);

    QScopedPointer<QPixmap> m_pPixmap;
    QScopedPointer<QSvgRenderer> m_pSvg;
    DrawMode m_draw_mode;
};

typedef QSharedPointer<Paintable> PaintablePointer;
typedef QWeakPointer<Paintable> WeakPaintablePointer;

class WPixmapStore {
  public:
    static PaintablePointer getPaintable(PixmapSource source,
                                            Paintable::DrawMode mode);
    static QPixmap* getPixmapNoCache(const QString& fileName);
    static void setLoader(QSharedPointer<ImgSource> ld);

  private:
    static QHash<QString, WeakPaintablePointer> m_paintableCache;
    static QSharedPointer<ImgSource> m_loader;
};

#endif
