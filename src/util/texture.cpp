#include "util/texture.h"

std::unique_ptr<QOpenGLTexture> createTexture(const QImage& image) {
    if (image.isNull()) {
        return std::unique_ptr<QOpenGLTexture>{};
    }
    std::unique_ptr<QOpenGLTexture> pTexture{new QOpenGLTexture(image)};
    pTexture->setMinificationFilter(QOpenGLTexture::Linear);
    pTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTexture->setWrapMode(QOpenGLTexture::ClampToEdge);

    return pTexture;
}

std::unique_ptr<QOpenGLTexture> createTexture(const QPixmap& pixmap) {
    return createTexture(pixmap.toImage());
}

std::unique_ptr<QOpenGLTexture> createTexture(const QSharedPointer<Paintable>& pPaintable) {
    if (pPaintable.isNull()) {
        return std::unique_ptr<QOpenGLTexture>{};
    }
    return createTexture(pPaintable->toImage());
}

std::unique_ptr<QOpenGLTexture> createTexture(const std::shared_ptr<QImage>& pImage) {
    if (!pImage) {
        return std::unique_ptr<QOpenGLTexture>{};
    }
    return createTexture(*pImage);
}
