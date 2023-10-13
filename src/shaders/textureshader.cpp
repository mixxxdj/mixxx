#include "shaders/textureshader.h"

using namespace mixxx;

void TextureShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform highp mat4 matrix;
attribute highp vec4 position;
attribute highp vec2 texcoor;
varying highp vec2 vTexcoor;
void main()
{
    vTexcoor = texcoor;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D sampler;
varying highp vec2 vTexcoor;
void main()
{
    gl_FragColor = texture2D(sampler, vTexcoor);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_samplerLocation = uniformLocation("sampler");
    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoor");
}
