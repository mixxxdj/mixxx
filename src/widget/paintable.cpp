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
        : m_drawMode(mode) {
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
    // Note: m_pPixmap is a QScopedPointer<QPixmap> and not a QPixmap.
    // This confusion let to the wrong assumption that we could simple
    //   return m_pPixmap->toImage();
    // relying on QPixmap returning QImage() when it was null.
    return m_pPixmap ? m_pPixmap->toImage() : QImage();
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
    // qDebug() << "Paintable::drawInternal" << DrawModeToString(m_drawMode)
    //          << targetRect << sourceRect;
    if (m_pSvg) {
        if (m_drawMode == TILE) {
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
        if (m_drawMode == TILE) {
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

void Paintable::mayCorrectColors() {
    if (WPixmapStore::willCorrectColors()) {
        QImage image = m_pPixmap->toImage();
        WPixmapStore::correctImageColors(&image);
        m_pPixmap->convertFromImage(image);
    }
}
