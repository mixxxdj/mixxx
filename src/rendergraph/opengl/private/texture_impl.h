#pragma once

#include <QOpenGLTexture>

#include "rendergraph/texture.h"

class rendergraph::Texture::Impl {
  public:
    Impl(Context& context, const QImage& image)
            : m_pTexture(new QOpenGLTexture(premultiplyAlpha(image))) {
        m_pTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        m_pTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    }

    QOpenGLTexture* glTexture() const {
        return m_pTexture.get();
    }

  private:
    const std::unique_ptr<QOpenGLTexture> m_pTexture{};

    static QImage premultiplyAlpha(const QImage& image) {
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
};
