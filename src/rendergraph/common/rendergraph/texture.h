#pragma once

#include <QImage>
#include <memory>

#include "backend/basetexture.h"

namespace rendergraph {
class Context;
class Texture;
} // namespace rendergraph

class rendergraph::Texture {
  public:
    Texture(Context& context, const QImage& image);

    BaseTexture* backendTexture() const {
        return m_pTexture.get();
    }

  private:
    const std::unique_ptr<BaseTexture> m_pTexture{};
};
