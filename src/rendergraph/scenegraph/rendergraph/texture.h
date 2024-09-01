#pragma once

#include <QImage>

namespace rendergraph {
class Context;
class Texture;
} // namespace rendergraph

class rendergraph::Texture {
  public:
    class Impl;

    Texture(Context& context, const QImage& image);
    ~Texture();
    Impl& impl() const;

  private:
    Texture(Impl* pImpl);

    const std::unique_ptr<Impl> m_pImpl;
};
