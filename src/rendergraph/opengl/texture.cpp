#include "rendergraph/texture.h"

#include <qnamespace.h>
#include <qrgb.h>

#include "rendergraph/assert.h"
#include "rendergraph/context.h"

using namespace rendergraph;

namespace {
QImage premultiplyAlpha(const QImage& image) {
    // Since the image is passed by const reference, implicit copy cannot be
    // used, and this Qimage::bits will return a ref the QImage buffer, which
    // may have a shorter lifecycle that the texture buffer. In order to
    // workaround this, and because we cannot copy the image as we need to use
    // the raw bitmap with an explicit image format, we make a manual copy of
    // the buffer
    QImage result(image.width(), image.height(), QImage::Format_RGBA8888);
    if (image.format() == QImage::Format_RGBA8888_Premultiplied) {
        VERIFY_OR_DEBUG_ASSERT(result.sizeInBytes() == image.sizeInBytes()) {
            result.fill(QColor(Qt::transparent).rgba());
            return result;
        }
        std::memcpy(result.bits(), image.bits(), result.sizeInBytes());
    } else {
        auto convertedImage = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
        VERIFY_OR_DEBUG_ASSERT(result.sizeInBytes() == convertedImage.sizeInBytes()) {
            result.fill(QColor(Qt::transparent).rgba());
            return result;
        }
        std::memcpy(result.bits(), convertedImage.bits(), result.sizeInBytes());
    }
    return result;
    /* TODO rendergraph ASK @acolombier to try if the following works as well
     * (added the .copy())
    if (image.format() == QImage::Format_RGBA8888_Premultiplied) {
        return QImage(image.bits(), image.width(), image.height(), QImage::Format_RGBA8888).copy();
    }
    return QImage(
            image.convertToFormat(QImage::Format_RGBA8888_Premultiplied)
                    .bits(),
            image.width(),
            image.height(),
            QImage::Format_RGBA8888).copy();
    */
}
} // namespace

Texture::Texture(Context*, const QImage& image)
        : m_pTexture(std::make_unique<QOpenGLTexture>(premultiplyAlpha(image))) {
    m_pTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_pTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
}

qint64 Texture::comparisonKey() const {
    return static_cast<qint64>(m_pTexture->textureId());
}
