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
    if (it == stringMap.end()) {
        qWarning() << "Unknown DrawMode string passed to DrawModeFromString:"
                   << str << "using DrawMode::Fixed as fallback";
        return DrawMode::Fixed;
    }

    return it.value();
}

// static
QString Paintable::DrawModeToString(DrawMode mode) {
    static const QMap<DrawMode, QString> modeMap = {
            {DrawMode::Fixed, "FIXED"},
            {DrawMode::Stretch, "STRETCH"},
            {DrawMode::StretchAspect, "STRETCH_ASPECT"},
            {DrawMode::Tile, "TILE"}};

    auto it = modeMap.find(mode);
    if (it == modeMap.end()) {
        qWarning() << "Unknown DrawMode passed to DrawModeToString "
                   << static_cast<int>(mode) << "using FIXED as fallback";
        DEBUG_ASSERT(false);
        return "FIXED";
    }

    return it.value();
}

Paintable::Paintable(const PixmapSource& source, DrawMode mode, double scaleFactor)
        : m_drawMode(mode) {
    if (!source.isSVG()) {
        auto pPixmap = WPixmapStore::getPixmapNoCache(source.getPath(), scaleFactor);
        if (!pPixmap) {
            qWarning() << "Failed to load pixmap from path:" << source.getPath();
            return;
        }
        m_pPixmap = std::move(pPixmap);
    } else {
        if (source.getPath().isEmpty()) {
            return;
        }
        auto pSvg = std::make_unique<QSvgRenderer>();
        if (!pSvg->load(source.getPath())) {
            return;
        }
        m_pSvg = std::move(pSvg);
    }
}

bool Paintable::isNull() const {
    return !(m_pPixmap || m_pSvg);
}

QSize Paintable::size() const {
    if (m_pSvg) {
        return m_pSvg->defaultSize();
    }
    if (m_pPixmap) {
        return m_pPixmap->size();
    }
    return QSize();
}

int Paintable::width() const {
    if (m_pSvg) {
        QSize size = m_pSvg->defaultSize();
        return size.width();
    }
    if (m_pPixmap) {
        return m_pPixmap->width();
    }
    return 0;
}

int Paintable::height() const {
    if (m_pSvg) {
        QSize size = m_pSvg->defaultSize();
        return size.height();
    }
    if (m_pPixmap) {
        return m_pPixmap->height();
    }
    return 0;
}

QRectF Paintable::rect() const {
    if (m_pSvg) {
        return QRectF(QPointF(0, 0), m_pSvg->defaultSize());
    }
    if (m_pPixmap) {
        return m_pPixmap->rect();
    }
    return QRectF();
}

QImage Paintable::toImage() const {
    if (m_pPixmap) {
        return m_pPixmap->toImage();
    }

    if (m_pSvg) {
        QImage image(m_pSvg->defaultSize(), QImage::Format_ARGB32_Premultiplied);
        // The constructor doesn't initialize the image with data,
        // so we need to fill it before we can draw on it.
        image.fill(Qt::transparent);
        QPainter painter(&image);
        m_pSvg->render(&painter);
        return image;
    }

    return QImage();
}

void Paintable::draw(const QRectF& targetRect, QPainter* pPainter) {
    // The sourceRect is implicitly the entire Paintable.
    draw(targetRect, pPainter, rect());
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
    // qDebug() << "Paintable::drawInternal" << DrawModeToString(m_drawMode)
    //          << targetRect << sourceRect;
    if (m_pSvg) {
        if (m_drawMode == DrawMode::Tile) {
            if (!m_pPixmap) {
                // qDebug() << "Paintable cache miss";
                qreal devicePixelRatio = pPainter->device()->devicePixelRatio();
                m_pPixmap = std::make_unique<QPixmap>(m_pSvg->defaultSize() * devicePixelRatio);
                m_pPixmap->setDevicePixelRatio(devicePixelRatio);
                m_pPixmap->fill(Qt::transparent);
                { // QPainter Scope
                    auto pixmapPainter = QPainter(m_pPixmap.get());
                    m_pSvg->render(&pixmapPainter);
                }
                mayCorrectColors();
            }
            // The SVG renderer doesn't directly support tiling, so we render
            // it to a pixmap which will then get tiled.
            pPainter->drawTiledPixmap(targetRect, *m_pPixmap);
        } else {
            if (!m_pPixmap ||
                    m_pPixmap->size() != targetRect.size().toSize() ||
                    m_lastSourceRect != sourceRect) {
                // qDebug() << "Paintable cache miss";
                qreal devicePixelRatio = pPainter->device()->devicePixelRatio();
                m_pPixmap = std::make_unique<QPixmap>(
                        targetRect.size().toSize() * devicePixelRatio);
                m_pPixmap->setDevicePixelRatio(devicePixelRatio);
                m_pPixmap->fill(Qt::transparent);
                { // QPainter Scope
                    auto pixmapPainter = QPainter(m_pPixmap.get());
                    QRectF deviceSourceRect = QRectF(
                            sourceRect.x() * devicePixelRatio,
                            sourceRect.y() * devicePixelRatio,
                            sourceRect.width() * devicePixelRatio,
                            sourceRect.height() * devicePixelRatio);
                    m_pSvg->setViewBox(deviceSourceRect);
                    m_pSvg->render(&pixmapPainter);
                }
                mayCorrectColors();
                m_lastSourceRect = sourceRect;
            }
            pPainter->drawPixmap(targetRect.topLeft(), *m_pPixmap);
        }
        return;
    }
    if (m_pPixmap) {
        // Note: Qt rounds the target rect to device pixels internally
        // using  roundInDeviceCoordinates()
        if (m_drawMode == DrawMode::Tile) {
            pPainter->drawTiledPixmap(targetRect, *m_pPixmap);
        } else {
            if (static_cast<QRectF>(m_pPixmap->rect()) == sourceRect &&
                    sourceRect.size() == targetRect.size()) {
                // Copy the whole pixmap without scaling
                pPainter->drawPixmap(targetRect.topLeft(), *m_pPixmap);
            } else {
                // qDebug() << "Drawing QPixmap scaled or chopped";
                // With scaling or chopping
                pPainter->drawPixmap(targetRect, *m_pPixmap, sourceRect);
            }
        }
        return;
    }
}


void Paintable::mayCorrectColors() {
    if (WPixmapStore::willCorrectColors()) {
        QImage image = m_pPixmap->toImage();
        WPixmapStore::correctImageColors(&image);
        m_pPixmap->convertFromImage(image);
    }
}
