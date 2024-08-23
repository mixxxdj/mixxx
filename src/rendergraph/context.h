#pragma once

#include <memory>

namespace rendergraph {
class Context;
}

class rendergraph::Context {
  public:
    class Impl;
    Context();
    ~Context();
    Impl& impl() const;

  private:
    const std::unique_ptr<Impl> m_pImpl;
};
