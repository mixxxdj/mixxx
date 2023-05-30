#include "util/texture.h"

QOpenGLTexture* createTexture(const QImage& image) {
    if (image.isNull()) {
        return nullptr;
    }
    QOpenGLTexture* pTexture = new QOpenGLTexture(image);
    pTexture->setMinificationFilter(QOpenGLTexture::Linear);
    pTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTexture->setWrapMode(QOpenGLTexture::ClampToBorder);

    return pTexture;
}

QOpenGLTexture* createTexture(const QPixmap& pixmap) {
    return createTexture(pixmap.toImage());
}

QOpenGLTexture* createTexture(const QSharedPointer<Paintable>& pPaintable) {
    if (pPaintable.isNull()) {
        return nullptr;
    }
    return createTexture(pPaintable->toImage());
}

QOpenGLTexture* createTexture(const std::shared_ptr<QImage>& pImage) {
    if (!pImage) {
        return nullptr;
    }
    return createTexture(*pImage);
}
