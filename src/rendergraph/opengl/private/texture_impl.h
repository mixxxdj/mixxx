#pragma once

#include <QOpenGLTexture>

#include "rendergraph/texture.h"

class rendergraph::Texture::Impl {
  public:
    Impl(Context& context, const QImage& image)
            : m_pTexture(new QOpenGLTexture(image.convertToFormat(
                      QImage::Format_ARGB32_Premultiplied))) {
    }

    QOpenGLTexture* glTexture() const {
        return m_pTexture.get();
    }

  private:
    const std::unique_ptr<QOpenGLTexture> m_pTexture{};
};
