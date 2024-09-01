#pragma once

#include <memory>

class QImage;
class QOpenGLTexture;

namespace rendergraph {
class Context;
class Texture;
} // namespace rendergraph

class rendergraph::Texture {
  public:
    Texture(Context& context, const QImage& image);
    ~Texture();

    QOpenGLTexture* glTexture() const;

  private:
    const std::unique_ptr<QOpenGLTexture> m_pTexture{};
    static QImage premultiplyAlpha(const QImage& image);
};
