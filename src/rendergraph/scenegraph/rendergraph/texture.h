#pragma once

#include <memory>

class QSGTexture;
class QImage;

namespace rendergraph {
class Context;
class Texture;
} // namespace rendergraph

// We can't use inheritance because QSGTexture has pure virtuals that we can't
// implement, so we encapsulate instead.
class rendergraph::Texture {
  public:
    Texture(Context& context, const QImage& image);
    ~Texture();

    QSGTexture* sgTexture() const {
        return m_pTexture.get();
    }

  private:
    std::unique_ptr<QSGTexture> m_pTexture{};
};
