#pragma once

#include <QSGTexture>

#include "rendergraph/texture.h"

// We can't use inheritance because QSGTexture has pure virtuals that we can't
// implement, so we encapsulate instead.
class rendergraph::Texture::Impl {
  public:
    Impl(Context& context, const QImage& image);

    QSGTexture* sgTexture() const {
        return m_pTexture.get();
    }

  private:
    std::unique_ptr<QSGTexture> m_pTexture{};
};
