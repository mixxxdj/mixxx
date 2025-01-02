#include "rendergraph/texture.h"

#include "rendergraph/assert.h"
#include "rendergraph/context.h"

using namespace rendergraph;

Texture::Texture(Context* pContext, const QImage& image)
        : m_pTexture(pContext->window()->createTextureFromImage(image)) {
    VERIFY_OR_DEBUG_ASSERT(pContext->window() != nullptr) {
        return;
    }
    DEBUG_ASSERT(!m_pTexture->textureSize().isNull());
}

qint64 Texture::comparisonKey() const {
    return m_pTexture->comparisonKey();
}
