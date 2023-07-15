#include "shaders/textureshader.h"

using namespace mixxx;

void TextureShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 texcoor;
varying vec3 vTexcoor;
void main()
{
    vTexcoor = texcoor;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D sampler;
varying vec3 vTexcoor;
void main()
{
    gl_FragColor = texture2D(sampler, vTexcoor.xy);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_samplerLocation = uniformLocation("sampler");
    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoor");
}
