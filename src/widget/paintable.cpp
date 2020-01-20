#include "widget/wpixmapstore.h"

#include <QDir>
#include <QString>
#include <QtDebug>

#include "skin/imgloader.h"

#include "util/math.h"
#include "util/memory.h"
#include "util/painterscope.h"

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

Paintable::Paintable(const PixmapSource& source, DrawMode mode, double scaleFactor)
        : m_drawMode(mode),
          m_source(source) {
    if (!source.isSVG()) {
        m_pPixmap.reset(WPixmapStore::getPixmapNoCache(source.getPath(), scaleFactor));
    } else {
        auto pSvg = std::make_unique<QSvgRenderer>();
        if (!source.getSvgSourceData().isEmpty()) {
            // Call here the different overload for svg content
            if (!pSvg->load(source.getSvgSourceData())) {
                // The above line already logs a warning
                return;
            }
        } else if (!source.getPath().isEmpty()) {
            if (!pSvg->load(source.getPath())) {
                // The above line already logs a warning
                return;
            }
        } else {
            return;
        }
        m_pSvg.reset(pSvg.release());
#ifdef __APPLE__
        // Apple does Retina scaling behind the scenes, so we also pass a
        // Paintable::FIXED image. On the other targets, it is better to
        // cache the pixmap. We do not do this for TILE and color schemas.
        // which can result in a correct but possibly blurry picture at a
        // Retina display. This can be fixed when switching to QT5
        if (mode == TILE || WPixmapStore::willCorrectColors()) {
#else
        if (mode == TILE || mode == Paintable::FIXED || WPixmapStore::willCorrectColors()) {
#endif
            // The SVG renderer doesn't directly support tiling, so we render
            // it to a pixmap which will then get tiled.
            QImage copy_buffer(m_pSvg->defaultSize() * scaleFactor, QImage::Format_ARGB32);
            copy_buffer.fill(0x00000000);  // Transparent black.
            QPainter painter(&copy_buffer);
            m_pSvg->render(&painter);
            WPixmapStore::correctImageColors(&copy_buffer);

            m_pPixmap.reset(new QPixmap(copy_buffer.size()));
            m_pPixmap->convertFromImage(copy_buffer);
        }
    }
}

bool Paintable::isNull() const {
    return m_source.isEmpty();
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

void Paintable::draw(const QRectF& targetRect, QPainter* pPainter,
                     const QRectF& sourceRect) {
    if (!targetRect.isValid() || !sourceRect.isValid() || isNull()) {
        return;
    }

    switch (m_drawMode) {
    case FIXED: {
        // Only render the minimum overlapping rectangle between the source
        // and target.
        QSizeF fixedSize(math_min(sourceRect.width(), targetRect.width()),
                         math_min(sourceRect.height(), targetRect.height()));
        QRectF adjustedTarget(targetRect.topLeft(), fixedSize);
        QRectF adjustedSource(sourceRect.topLeft(), fixedSize);
        drawInternal(adjustedTarget, pPainter, adjustedSource);
        break;
    }
    case STRETCH_ASPECT: {
        qreal sx = targetRect.width() / sourceRect.width();
        qreal sy = targetRect.height() / sourceRect.height();

        // Adjust the scale so that the scaling in both axes is equal.
        if (sx != sy) {
            qreal scale = math_min(sx, sy);
            QRectF adjustedTarget(targetRect.x(),
                                  targetRect.y(),
                                  scale * sourceRect.width(),
                                  scale * sourceRect.height());
            drawInternal(adjustedTarget, pPainter, sourceRect);
        } else {
            drawInternal(targetRect, pPainter, sourceRect);
        }
        break;
    }
    case STRETCH:
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    case TILE:
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    }
}

void Paintable::drawCentered(const QRectF& targetRect, QPainter* pPainter,
                             const QRectF& sourceRect) {
    switch (m_drawMode) {
    case FIXED: {
        // Only render the minimum overlapping rectangle between the source
        // and target.
        QSizeF fixedSize(math_min(sourceRect.width(), targetRect.width()),
                         math_min(sourceRect.height(), targetRect.height()));

        QRectF adjustedSource(sourceRect.topLeft(), fixedSize);
        QRectF adjustedTarget(QPointF(-adjustedSource.width() / 2.0,
                                      -adjustedSource.height() / 2.0),
                              fixedSize);
        drawInternal(adjustedTarget, pPainter, adjustedSource);
        break;
    }
    case STRETCH_ASPECT: {
        qreal sx = targetRect.width() / sourceRect.width();
        qreal sy = targetRect.height() / sourceRect.height();

        // Adjust the scale so that the scaling in both axes is equal.
        if (sx != sy) {
            qreal scale = math_min(sx, sy);
            qreal scaledWidth = scale * sourceRect.width();
            qreal scaledHeight = scale * sourceRect.height();
            QRectF adjustedTarget(-scaledWidth / 2.0, -scaledHeight / 2.0,
                                  scaledWidth, scaledHeight);
            drawInternal(adjustedTarget, pPainter, sourceRect);
        } else {
            drawInternal(targetRect, pPainter, sourceRect);
        }
        break;
    }
    case STRETCH:
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    case TILE:
        // TODO(XXX): What's the right behavior here? Draw the first tile at the
        // center point and then tile all around it based on that?
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    }
}

void Paintable::drawInternal(const QRectF& targetRect, QPainter* pPainter,
                             const QRectF& sourceRect) {
    // qDebug() << "Paintable::drawInternal" << DrawModeToString(m_draw_mode)
    //          << targetRect << sourceRect;
    if (m_pPixmap) {
        if (m_drawMode == TILE) {
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
        if (m_drawMode == TILE) {
            qWarning() << "Tiled SVG should have been rendered to pixmap!";
        } else {
            // NOTE(rryan): QSvgRenderer render does not clip for us -- it
            // applies a world transformation using viewBox and renders the
            // entire SVG to the painter. We save/restore the QPainter in case
            // there is an existing clip region (I don't know of any Mixxx code
            // that uses one but we may in the future).
            PainterScope PainterScope(pPainter);
            pPainter->setClipping(true);
            pPainter->setClipRect(targetRect);
            m_pSvg->setViewBox(sourceRect);
            m_pSvg->render(pPainter, targetRect);
        }
    }
}

// static
QString Paintable::getAltFileName(const QString& fileName) {
    // Detect if the alternate image file exists and, if it does,
    // return its path instead
    QStringList temp = fileName.split('.');
    if (temp.length() != 2) {
        return fileName;
    }

    QString newFileName = temp[0] + QLatin1String("@2x.") + temp[1];
    QFile file(newFileName);
    if (QFileInfo(file).exists()) {
        return newFileName;
    } else {
        return fileName;
    }
}
