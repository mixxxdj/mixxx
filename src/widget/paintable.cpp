#include "widget/paintable.h"

#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QSvgRenderer>
#include <QtDebug>
#include <memory>

#include "util/math.h"
#include "util/painterscope.h"
#include "widget/wpixmapstore.h"

// static
Paintable::DrawMode Paintable::DrawModeFromString(const QString& str) {
    static const QMap<QString, DrawMode> stringMap = {
            {"FIXED", DrawMode::Fixed},
            {"STRETCH", DrawMode::Stretch},
            {"STRETCH_ASPECT", DrawMode::StretchAspect},
            {"TILE", DrawMode::Tile}};

    auto it = stringMap.find(str.toUpper());
    if (it != stringMap.end()) {
        return it.value();
    } else {
        qWarning() << "Unknown DrawMode string passed to DrawModeFromString:"
                   << str << "using DrawMode::Fixed as fallback";
        return DrawMode::Fixed;
    }
}

// static
QString Paintable::DrawModeToString(DrawMode mode) {
    static const QMap<DrawMode, QString> modeMap = {
            {DrawMode::Fixed, "FIXED"},
            {DrawMode::Stretch, "STRETCH"},
            {DrawMode::StretchAspect, "STRETCH_ASPECT"},
            {DrawMode::Tile, "TILE"}};

    auto it = modeMap.find(mode);
    if (it != modeMap.end()) {
        return it.value();
    } else {
        qWarning() << "Unknown DrawMode passed to DrawModeToString "
                   << static_cast<int>(mode) << "using FIXED as fallback";
        DEBUG_ASSERT(false);
        return "FIXED";
    }
}

Paintable::Paintable(const PixmapSource& source, DrawMode mode, double scaleFactor)
        : m_drawMode(mode),
          m_source(source) {
    if (!source.isSVG()) {
            auto pPixmap = WPixmapStore::getPixmapNoCache(source.getPath(), scaleFactor);
            if (!pPixmap) {
                qWarning() << "Failed to load pixmap from path:" << source.getPath();
                return;
            }
            m_pPixmap = std::move(pPixmap);
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
        // DrawMode::Fixed image. On the other targets, it is better to
        // cache the pixmap. We do not do this for Tile and color schemas.
        // which can result in a correct but possibly blurry picture at a
        // Retina display. This can be fixed when switching to QT5
        if (mode == DrawMode::Tile || WPixmapStore::willCorrectColors()) {
#else
        if (mode == DrawMode::Tile || mode == DrawMode::Fixed ||
                WPixmapStore::willCorrectColors()) {
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
    if (m_pPixmap) {
        return m_pPixmap->size();
    } else if (m_pSvg) {
        return m_pSvg->defaultSize();
    }
    return QSize();
}

int Paintable::width() const {
    if (m_pPixmap) {
        return m_pPixmap->width();
    } else if (m_pSvg) {
        QSize size = m_pSvg->defaultSize();
        return size.width();
    }
    return 0;
}

int Paintable::height() const {
    if (m_pPixmap) {
        return m_pPixmap->height();
    } else if (m_pSvg) {
        QSize size = m_pSvg->defaultSize();
        return size.height();
    }
    return 0;
}

QRectF Paintable::rect() const {
    if (m_pPixmap) {
        return m_pPixmap->rect();
    } else if (m_pSvg) {
        return QRectF(QPointF(0, 0), m_pSvg->defaultSize());
    }
    return QRectF();
}

QImage Paintable::toImage() const {
    // Note: m_pPixmap is a QScopedPointer<QPixmap> and not a QPixmap.
    // This confusion let to the wrong assumption that we could simple
    //   return m_pPixmap->toImage();
    // relying on QPixmap returning QImage() when it was null.
    return !m_pPixmap ? QImage() : m_pPixmap->toImage();
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
    case DrawMode::Fixed: {
        // Only render the minimum overlapping rectangle between the source
        // and target.
        QSizeF fixedSize(math_min(sourceRect.width(), targetRect.width()),
                         math_min(sourceRect.height(), targetRect.height()));
        QRectF adjustedTarget(targetRect.topLeft(), fixedSize);
        QRectF adjustedSource(sourceRect.topLeft(), fixedSize);
        drawInternal(adjustedTarget, pPainter, adjustedSource);
        break;
    }
    case DrawMode::StretchAspect: {
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
    case DrawMode::Stretch:
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    case DrawMode::Tile:
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    }
}

void Paintable::drawCentered(const QRectF& targetRect, QPainter* pPainter,
                             const QRectF& sourceRect) {
    switch (m_drawMode) {
    case DrawMode::Fixed: {
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
    case DrawMode::StretchAspect: {
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
    case DrawMode::Stretch:
        drawInternal(targetRect, pPainter, sourceRect);
        break;
    case DrawMode::Tile:
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
        if (m_drawMode == DrawMode::Tile) {
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
        if (m_drawMode == DrawMode::Tile) {
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
