#pragma once

#include <memory>

namespace rendergraph {
class AttributeSet;
class UniformSet;
class MaterialShader;
} // namespace rendergraph

class rendergraph::MaterialShader {
  public:
    class Impl;

    MaterialShader(const char* vertexShaderFile,
            const char* fragmentShaderFile,
            const UniformSet& uniforms,
            const AttributeSet& attributeSet);
    ~MaterialShader();
    Impl& impl() const;

  private:
    MaterialShader(Impl* pImpl);

    const std::unique_ptr<Impl> m_pImpl;
};
