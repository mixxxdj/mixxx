#include "util/opengltexture2d.h"

#include <QPixmap>

#include "widget/paintable.h"

OpenGLTexture2D::OpenGLTexture2D()
        : QOpenGLTexture(QOpenGLTexture::Target2D){};

void OpenGLTexture2D::setData(const QImage& image) {
    destroy();
    if (!image.isNull()) {
        QOpenGLTexture::setData(image);
        setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        setWrapMode(QOpenGLTexture::ClampToEdge);
    }
};

void OpenGLTexture2D::setData(const QPixmap& pixmap) {
    setData(pixmap.toImage());
};

void OpenGLTexture2D::setData(const QSharedPointer<Paintable>& pPaintable) {
    if (pPaintable) {
        setData(pPaintable->toImage());
    }
};

void OpenGLTexture2D::setData(const std::shared_ptr<QImage>& pImage) {
    if (pImage) {
        setData(*pImage);
    }
};
