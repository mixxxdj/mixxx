#include "rendergraph/texture.h"

#include <QOpenGLTexture>

using namespace rendergraph;

Texture::Texture(Context& context, const QImage& image)
        : m_pTexture(new QOpenGLTexture(premultiplyAlpha(image))) {
    m_pTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_pTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
}

Texture::~Texture() = default;

QOpenGLTexture* Texture::glTexture() const {
    return m_pTexture.get();
}

QImage Texture::premultiplyAlpha(const QImage& image) {
    if (image.format() == QImage::Format_RGBA8888_Premultiplied) {
        return QImage(image.bits(), image.width(), image.height(), QImage::Format_RGBA8888);
    }
    return QImage(
            image.convertToFormat(QImage::Format_RGBA8888_Premultiplied)
                    .bits(),
            image.width(),
            image.height(),
            QImage::Format_RGBA8888);
}
