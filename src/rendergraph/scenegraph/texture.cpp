#include "rendergraph/texture.h"
#include "rendergraph/context.h"

using namespace rendergraph;

Texture::Texture(Context& context, const QImage& image)
        : m_pTexture(context.window()->createTextureFromImage(image)) {
}
