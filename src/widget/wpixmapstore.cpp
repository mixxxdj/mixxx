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

#include <QtDebug>

// static
QHash<QString, WeakPaintablePointer> WPixmapStore::m_paintableCache;
QSharedPointer<ImgSource> WPixmapStore::m_loader = QSharedPointer<ImgSource>();

Paintable::Paintable(QImage* pImage) {
#if QT_VERSION >= 0x040700
    m_pPixmap.reset(new QPixmap());
    m_pPixmap->convertFromImage(*pImage);
#else
    m_pPixmap.reset(new QPixmap(QPixmap::fromImage(*pImage)));
#endif
    delete pImage;
}

Paintable::Paintable(const QString& fileName) {
    if (fileName.endsWith(".svg", Qt::CaseInsensitive)) {
        m_pSvg.reset(new QSvgRenderer(fileName));
    } else {
        m_pPixmap.reset(new QPixmap(fileName));
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

void Paintable::draw(const QRectF& targetRect, QPainter* pPainter) {
    if (!targetRect.isValid()) {
        return;
    }

    if (!m_pPixmap.isNull() && !m_pPixmap->isNull()) {
        pPainter->drawPixmap(targetRect, *m_pPixmap, m_pPixmap->rect());
    } else if (!m_pSvg.isNull() && m_pSvg->isValid()) {
        m_pSvg->render(pPainter, targetRect);
    }
}

void Paintable::drawTiled(const QRectF& targetRect, QPainter* pPainter) {
    if (!targetRect.isValid()) {
        return;
    }

    if (!m_pPixmap.isNull() && !m_pPixmap->isNull()) {
        pPainter->drawTiledPixmap(targetRect, *m_pPixmap, QPoint(0,0));
    } else if (!m_pSvg.isNull() && m_pSvg->isValid()) {
        m_pSvg->render(pPainter, targetRect);
    }
}


void Paintable::draw(const QRectF& targetRect, QPainter* pPainter,
                     const QRectF& sourceRect) {
    if (!targetRect.isValid() || !sourceRect.isValid()) {
        return;
    }

    if (m_pPixmap && !m_pPixmap->isNull()) {
        pPainter->drawPixmap(targetRect, *m_pPixmap, sourceRect);
    } else if (m_pSvg && m_pSvg->isValid()) {
        resizeSvgPixmap(targetRect, sourceRect);

        QRectF newSource(m_pPixmapSvg->width() * sourceRect.x() / sourceRect.width(),
                         m_pPixmapSvg->height() * sourceRect.y() / sourceRect.height(),
                         targetRect.width(), targetRect.height());
        pPainter->drawPixmap(targetRect, *m_pPixmapSvg, newSource);
    }
}

void Paintable::draw(int x, int y, QPainter* pPainter) {
    if (m_pPixmap && !m_pPixmap->isNull()) {
        pPainter->drawPixmap(x, y, *m_pPixmap);
    } else if (m_pSvg && m_pSvg->isValid()) {
        QRectF targetRect(QPointF(x, y), m_pSvg->defaultSize());
        m_pSvg->render(pPainter, targetRect);
    }
}

void Paintable::draw(const QPointF& point, QPainter* pPainter, const QRectF& sourceRect) {
    return draw(QRectF(point, sourceRect.size()), pPainter, sourceRect);
}

void Paintable::resizeSvgPixmap(const QRectF& targetRect,
                                const QRectF& sourceRect) {
    if (m_pSvg.isNull() || !m_pSvg->isValid()) {
        return;
    }

    double sx = targetRect.width() / sourceRect.width();
    double sy = targetRect.height() / sourceRect.height();

    QSize originalSize = m_pSvg->defaultSize();
    QSize projectedSize(originalSize.width() * sx,
                        originalSize.height() * sy);

    if (m_pPixmapSvg.isNull() || m_pPixmapSvg->size() != projectedSize) {
        m_pPixmapSvg.reset(new QPixmap(projectedSize));
        QPainter pixmapPainter(m_pPixmapSvg.data());
        m_pSvg->render(&pixmapPainter);
    }
}

// static
PaintablePointer WPixmapStore::getPaintable(const QString& fileName) {
    // See if we have a cached value for the pixmap.
    PaintablePointer pPaintable = m_paintableCache.value(fileName, PaintablePointer());
    if (pPaintable) {
        return pPaintable;
    }

    // Otherwise, construct it with the pixmap loader.
    //qDebug() << "WPixmapStore Loading pixmap from file" << fileName;

    if (m_loader) {
        QImage* pImage = m_loader->getImage(fileName);
        pPaintable = PaintablePointer(new Paintable(pImage));
    } else {
        pPaintable = PaintablePointer(new Paintable(fileName));
    }

    if (pPaintable.isNull() || pPaintable->isNull()) {
        qDebug() << "WPixmapStore couldn't load:" << fileName
                 << pPaintable.isNull();
        return PaintablePointer();
    }
    m_paintableCache[fileName] = pPaintable;
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
