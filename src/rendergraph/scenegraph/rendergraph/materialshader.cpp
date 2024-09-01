#include "rendergraph/materialshader.h"

#include "materialshader_impl.h"

using namespace rendergraph;

MaterialShader::MaterialShader(Impl* pImpl)
        : m_pImpl(pImpl) {
}

MaterialShader::MaterialShader(const char* vertexShaderFile,
        const char* fragmentShaderFile,
        const UniformSet& uniformSet,
        const AttributeSet& attributeSet)
        : MaterialShader(new MaterialShader::Impl(this,
                  vertexShaderFile,
                  fragmentShaderFile,
                  uniformSet,
                  attributeSet)){};

MaterialShader::~MaterialShader() = default;

MaterialShader::Impl& MaterialShader::impl() const {
    return *m_pImpl;
}
