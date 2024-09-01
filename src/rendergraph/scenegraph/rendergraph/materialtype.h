#pragma once

#include <memory>

namespace rendergraph {
class MaterialType;
}

class rendergraph::MaterialType {
  public:
    class Impl;

    MaterialType();
    ~MaterialType();
    Impl& impl() const;

  private:
    MaterialType(Impl* pImpl);

    const std::unique_ptr<Impl> m_pImpl;
};
