#include "rendergraph/texture.h"

#include <QImage>
#include <QSGTexture>

#include "rendergraph/context.h"

using namespace rendergraph;

Texture::Texture(Context& context, const QImage& image)
        : m_pTexture(context.window()->createTextureFromImage(image)) {
}

Texture::~Texture() = default;
