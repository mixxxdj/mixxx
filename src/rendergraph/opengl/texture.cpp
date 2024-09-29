#include "rendergraph/texture.h"

#include "rendergraph/context.h"

using namespace rendergraph;

namespace {
QImage premultiplyAlpha(const QImage& image) {
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
} // namespace

Texture::Texture(Context& context, const QImage& image)
        : m_pTexture(std::make_unique<QOpenGLTexture>(premultiplyAlpha(image))) {
    m_pTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_pTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
}

qint64 Texture::comparisonKey() const {
    return static_cast<qint64>(m_pTexture->textureId());
}
