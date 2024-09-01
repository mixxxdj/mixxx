#include "rendergraph/texture.h"

#include "texture_impl.h"

using namespace rendergraph;

Texture::Texture(Impl* pImpl)
        : m_pImpl(pImpl) {
}

Texture::Texture(Context& context, const QImage& image)
        : Texture(new Texture::Impl(context, image)) {
}

Texture::~Texture() = default;

Texture::Impl& Texture::impl() const {
    return *m_pImpl;
}
