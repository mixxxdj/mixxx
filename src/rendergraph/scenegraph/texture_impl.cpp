#include "texture_impl.h"

#include "context_impl.h"

using namespace rendergraph;

Texture::Impl::Impl(Context& context, const QImage& image)
        : m_pTexture(context.impl().window()->createTextureFromImage(image)) {
}
