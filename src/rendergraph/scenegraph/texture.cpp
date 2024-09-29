#include "rendergraph/texture.h"
#include "rendergraph/context.h"

using namespace rendergraph;

Texture::Texture(Context& context, const QImage& image)
        : m_pTexture(context.window()->createTextureFromImage(image)) {
}

qint64 Texture::comparisonKey() const {
    return m_pTexture->comparisonKey();
}
