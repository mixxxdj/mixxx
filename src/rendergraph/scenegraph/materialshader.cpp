#include "rendergraph/materialshader.h"

using namespace rendergraph;

MaterialShader::MaterialShader(const char* vertexShaderFile,
        const char* fragmentShaderFile,
        const UniformSet& uniformSet,
        const AttributeSet& attributeSet) {
    (void)uniformSet;
    (void)attributeSet;
    setShaderFileName(VertexStage, resource(vertexShaderFile));
    setShaderFileName(FragmentStage, resource(fragmentShaderFile));
}
