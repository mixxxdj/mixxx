#pragma once

#include <QImage>
#include <memory>

#include "backend/texture.h"

namespace rendergraph {
class Context;
class Texture;
} // namespace rendergraph

class rendergraph::Texture {
  public:
    Texture(Context& context, const QImage& image);

    backend::Texture* backendTexture() const {
        return m_pTexture.get();
    }

  private:
    const std::unique_ptr<backend::Texture> m_pTexture{};
};
