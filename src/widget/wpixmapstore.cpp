/***************************************************************************
                          wpixmapstore.cpp  -  description
                             -------------------
    begin                : Mon Jul 28 2003
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

#include "widget/wpixmapstore.h"

#include <QString>
#include <QtDebug>

#include "util/math.h"

// static
QHash<QString, WeakPaintablePointer> WPixmapStore::m_paintableCache;
QSharedPointer<ImgSource> WPixmapStore::m_loader = QSharedPointer<ImgSource>();

// static
Paintable::DrawMode Paintable::DrawModeFromString(const QString& str) {
    if (str.compare("FIXED", Qt::CaseInsensitive) == 0) {
        return FIXED;
    } else if (str.compare("STRETCH", Qt::CaseInsensitive) == 0) {
        return STRETCH;
    } else if (str.compare("STRETCH_ASPECT", Qt::CaseInsensitive) == 0) {
        return STRETCH_ASPECT;
    } else if (str.compare("TILE", Qt::CaseInsensitive) == 0) {
        return TILE;
    }

    // Fall back on the implicit default from before Mixxx supported draw modes.
    qWarning() << "Unknown DrawMode string in DrawModeFromString:"
               << str << "using FIXED";
    return FIXED;
}

// static
QString Paintable::DrawModeToString(DrawMode mode) {
    switch (mode) {
        case FIXED:
            return "FIXED";
        case STRETCH:
            return "STRETCH";
        case STRETCH_ASPECT:
            return "STRETCH_ASPECT";
        case TILE:
            return "TILE";
    }
    // Fall back on the implicit default from before Mixxx supported draw modes.
    qWarning() << "Unknown DrawMode in DrawModeToString " << mode
               << "using FIXED";
    return "FIXED";
}

Paintable::Paintable(QImage* pImage, DrawMode mode)
        : m_draw_mode(mode) {
#if QT_VERSION >= 0x040700
    m_pPixmap.reset(new QPixmap());
    m_pPixmap->convertFromImage(*pImage);
#else
    m_pPixmap.reset(new QPixmap(QPixmap::fromImage(*pImage)));
#endif
    delete pImage;
}

Paintable::Paintable(const QString& fileName, DrawMode mode)
        : m_draw_mode(mode) {
    if (fileName.endsWith(".svg", Qt::CaseInsensitive)) {
        if (mode == TILE) {
            // The SVG renderer doesn't directly support tiling, so we render
            // it to a pixmap which will then get tiled.
            QSvgRenderer renderer(fileName);
            QImage copy_buffer(renderer.defaultSize(), QImage::Format_ARGB32);
            copy_buffer.fill(0x00000000);  // Transparent black.
            m_pPixmap.reset(new QPixmap(renderer.defaultSize()));
            QPainter painter(&copy_buffer);
            renderer.render(&painter);
            m_pPixmap->convertFromImage(copy_buffer);
        } else {
            m_pSvg.reset(new QSvgRenderer(fileName));
        }
    } else {
        m_pPixmap.reset(new QPixmap(fileName));
    }
}

Paintable::Paintable(const PixmapSource& source, DrawMode mode)
        : m_draw_mode(mode) {
    if (source.isSVG()) {
        QScopedPointer<QSvgRenderer> pSvgRenderer(new QSvgRenderer());
        if (source.getData().isEmpty()) {
            pSvgRenderer->load(source.getPath());
        } else {
            pSvgRenderer->load(source.getData());
        }

        if (mode == TILE) {
            // The SVG renderer doesn't directly support tiling, so we render
            // it to a pixmap which will then get tiled.
            QImage copy_buffer(pSvgRenderer->defaultSize(), QImage::Format_ARGB32);
            copy_buffer.fill(0x00000000);  // Transparent black.
            m_pPixmap.reset(new QPixmap(pSvgRenderer->defaultSize()));
            QPainter painter(&copy_buffer);
            pSvgRenderer->render(&painter);
            m_pPixmap->convertFromImage(copy_buffer);
        } else {
            m_pSvg.reset(pSvgRenderer.take());
        }
    } else {
        QPixmap * pPixmap = new QPixmap();
        if (!source.getData().isEmpty()) {
            pPixmap->loadFromData(source.getData());
        } else {
            pPixmap->load(source.getPath());
        }
        m_pPixmap.reset(pPixmap);
    }
}


bool Paintable::isNull() const {
    if (!m_pPixmap.isNull()) {
        return m_pPixmap->isNull();
    } else if (!m_pSvg.isNull()) {
        return !m_pSvg->isValid();
    }
    return false;
}

QSize Paintable::size() const {
    if (!m_pPixmap.isNull()) {
        return m_pPixmap->size();
    } else if (!m_pSvg.isNull()) {
        return m_pSvg->defaultSize();
    }
    return QSize();
}

int Paintable::width() const {
    if (!m_pPixmap.isNull()) {
        return m_pPixmap->width();
    } else if (!m_pSvg.isNull()) {
        QSize size = m_pSvg->defaultSize();
        return size.width();
    }
    return 0;
}

int Paintable::height() const {
    if (!m_pPixmap.isNull()) {
        return m_pPixmap->height();
    } else if (!m_pSvg.isNull()) {
        QSize size = m_pSvg->defaultSize();
        return size.height();
    }
    return 0;
}

QRectF Paintable::rect() const {
    if (!m_pPixmap.isNull()) {
        return m_pPixmap->rect();
    } else if (!m_pSvg.isNull()) {
        return QRectF(QPointF(0, 0), m_pSvg->defaultSize());
    }
    return QRectF();
}

void Paintable::draw(const QRectF& targetRect, QPainter* pPainter) {
    // The sourceRect is implicitly the entire Paintable.
    draw(targetRect, pPainter, rect());
}

void Paintable::draw(int x, int y, QPainter* pPainter) {
    QRectF sourceRect(rect());
    QRectF targetRect(QPointF(x, y), sourceRect.size());
    draw(targetRect, pPainter, sourceRect);
}

void Paintable::draw(const QPointF& point, QPainter* pPainter, const QRectF& sourceRect) {
    return draw(QRectF(point, sourceRect.size()), pPainter, sourceRect);
}

void Paintable::draw(const QRectF& targetRect, QPainter* pPainter,
                     const QRectF& sourceRect) {
    if (!targetRect.isValid() || !sourceRect.isValid() || isNull()) {
        return;
    }

    if (m_draw_mode == FIXED) {
        // Only render the minimum overlapping rectangle between the source
        // and target.
        QSizeF fixedSize(math_min(sourceRect.width(), targetRect.width()),
                         math_min(sourceRect.height(), targetRect.height()));
        QRectF adjustedTarget(targetRect.topLeft(), fixedSize);
        QRectF adjustedSource(sourceRect.topLeft(), fixedSize);
        return drawInternal(adjustedTarget, pPainter, adjustedSource);
    } else if (m_draw_mode == STRETCH_ASPECT) {
        qreal sx = targetRect.width() / sourceRect.width();
        qreal sy = targetRect.height() / sourceRect.height();

        // Adjust the scale so that the scaling in both axes is equal.
        if (sx != sy) {
            qreal scale = math_min(sx, sy);
            QRectF adjustedTarget(targetRect.x(),
                                  targetRect.y(),
                                  scale * sourceRect.width(),
                                  scale * sourceRect.height());
            return drawInternal(adjustedTarget, pPainter, sourceRect);
        } else {
            return drawInternal(targetRect, pPainter, sourceRect);
        }
    } else if (m_draw_mode == STRETCH) {
        return drawInternal(targetRect, pPainter, sourceRect);
    } else if (m_draw_mode == TILE) {
        return drawInternal(targetRect, pPainter, sourceRect);
    }
}

void Paintable::drawCentered(const QRectF& targetRect, QPainter* pPainter,
                             const QRectF& sourceRect) {
    if (m_draw_mode == FIXED) {
        // Only render the minimum overlapping rectangle between the source
        // and target.
        QSizeF fixedSize(math_min(sourceRect.width(), targetRect.width()),
                         math_min(sourceRect.height(), targetRect.height()));

        QRectF adjustedSource(sourceRect.topLeft(), fixedSize);
        QRectF adjustedTarget(QPointF(-adjustedSource.width() / 2.0,
                                      -adjustedSource.height() / 2.0),
                              fixedSize);
        return drawInternal(adjustedTarget, pPainter, adjustedSource);
    } else if (m_draw_mode == STRETCH_ASPECT) {
        qreal sx = targetRect.width() / sourceRect.width();
        qreal sy = targetRect.height() / sourceRect.height();

        // Adjust the scale so that the scaling in both axes is equal.
        if (sx != sy) {
            qreal scale = math_min(sx, sy);
            qreal scaledWidth = scale * sourceRect.width();
            qreal scaledHeight = scale * sourceRect.height();
            QRectF adjustedTarget(-scaledWidth / 2.0, -scaledHeight / 2.0,
                                  scaledWidth, scaledHeight);
            return drawInternal(adjustedTarget, pPainter, sourceRect);
        } else {
            return drawInternal(targetRect, pPainter, sourceRect);
        }
    } else if (m_draw_mode == STRETCH) {
        return drawInternal(targetRect, pPainter, sourceRect);
    } else if (m_draw_mode == TILE) {
        // TODO(XXX): What's the right behavior here? Draw the first tile at the
        // center point and then tile all around it based on that?
        return drawInternal(targetRect, pPainter, sourceRect);
    }
}

void Paintable::drawInternal(const QRectF& targetRect, QPainter* pPainter,
                             const QRectF& sourceRect) {
    // qDebug() << "Paintable::drawInternal" << DrawModeToString(m_draw_mode)
    //          << targetRect << sourceRect;
    if (m_pPixmap) {
        if (m_draw_mode == TILE) {
            // TODO(rryan): Using a source rectangle doesn't make much sense
            // with tiling. Ignore the source rect and tile our natural size
            // across the target rect. What's the right general behavior here?
            // NOTE(rryan): We round our target/source rectangles to the nearest
            // pixel for raster images.
            pPainter->drawTiledPixmap(targetRect.toRect(), *m_pPixmap, QPoint(0,0));
        } else {
            // NOTE(rryan): We round our target/source rectangles to the nearest
            // pixel for raster images.
            pPainter->drawPixmap(targetRect.toRect(), *m_pPixmap,
                                 sourceRect.toRect());
        }
    } else if (m_pSvg) {
        if (m_draw_mode == TILE) {
            qWarning() << "Tiled SVG should have been rendered to pixmap!";
        } else {
            // NOTE(rryan): QSvgRenderer render does not clip for us -- it
            // applies a world transformation using viewBox and renders the
            // entire SVG to the painter. We save/restore the QPainter in case
            // there is an existing clip region (I don't know of any Mixxx code
            // that uses one but we may in the future).
            pPainter->save();
            pPainter->setClipping(true);
            pPainter->setClipRect(targetRect);
            m_pSvg->setViewBox(sourceRect);
            m_pSvg->render(pPainter, targetRect);
            pPainter->restore();
        }
    }
}

// static
PaintablePointer WPixmapStore::getPaintable(PixmapSource source,
                                            Paintable::DrawMode mode) {
    // See if we have a cached value for the pixmap.
    PaintablePointer pPaintable = m_paintableCache.value(source.getId(), PaintablePointer());
    if (pPaintable) {
        return pPaintable;
    }

    // Otherwise, construct it with the pixmap loader.
    //qDebug() << "WPixmapStore Loading pixmap from file" << source.getPath();

    if (m_loader) {
        QImage* pImage = m_loader->getImage(source.getPath());
        pPaintable = PaintablePointer(new Paintable(pImage, mode));
    } else {
        pPaintable = PaintablePointer(new Paintable(source, mode));
    }

    if (pPaintable.isNull() || pPaintable->isNull()) {
        // Only log if it looks like the user tried to specify a
        // pixmap. Otherwise we probably just have a widget that is calling
        // getPaintable without checking that the skinner actually wanted one.
        if (!source.isEmpty()) {
            qDebug() << "WPixmapStore couldn't load:" << source.getPath()
                     << pPaintable.isNull();
        }
        return PaintablePointer();
    }

    m_paintableCache[source.getId()] = pPaintable;
    return pPaintable;
}




// static
QPixmap* WPixmapStore::getPixmapNoCache(const QString& fileName) {
    QPixmap* pPixmap = NULL;
    if (m_loader) {
        QImage* img = m_loader->getImage(fileName);
#if QT_VERSION >= 0x040700
        pPixmap = new QPixmap();
        pPixmap->convertFromImage(*img);
#else
        pPixmap = new QPixmap(QPixmap::fromImage(*img));
#endif
        delete img;
    } else {
        pPixmap = new QPixmap(fileName);
    }
    return pPixmap;
}

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_paintableCache.clear();
}
