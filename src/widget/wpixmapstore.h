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

#include "skin/imgsource.h"

// Wrapper around QImage and QSvgRenderer to support rendering SVG images in
// high fidelity.
class Paintable {
  public:
    // Takes ownership of QImage.
    Paintable(QImage* pImage);
    Paintable(const QString& fileName);

    QSize size() const;
    int width() const;
    int height() const;

    void draw(int x, int y, QPainter* pPainter);
    void draw(const QPointF& point, QPainter* pPainter,
              const QRectF& sourceRect);
    void draw(const QRectF& targetRect, QPainter* pPainter);
    void drawTiled(const QRectF& targetRect, QPainter* pPainter);
    void draw(const QRectF& targetRect, QPainter* pPainter,
              const QRectF& sourceRect);
    bool isNull() const;

  private:
    void resizeSvgPixmap(const QRectF& targetRect, const QRectF& sourceRect);

    QScopedPointer<QPixmap> m_pPixmap;
    QScopedPointer<QSvgRenderer> m_pSvg;
    QScopedPointer<QPixmap> m_pPixmapSvg;
};

typedef QSharedPointer<Paintable> PaintablePointer;
typedef QWeakPointer<Paintable> WeakPaintablePointer;

class WPixmapStore {
  public:
    static PaintablePointer getPaintable(const QString& fileName);
    static QPixmap* getPixmapNoCache(const QString& fileName);
    static void setLoader(QSharedPointer<ImgSource> ld);

  private:
    static QHash<QString, WeakPaintablePointer> m_paintableCache;
    static QSharedPointer<ImgSource> m_loader;
};

#endif
