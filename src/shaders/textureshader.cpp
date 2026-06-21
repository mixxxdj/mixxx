#include "shaders/textureshader.h"

using namespace mixxx;

void TextureShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
#version 310 es
precision mediump float;
uniform highp mat4 matrix;
in highp vec4 position; // use vec4 here (will be padded) for matrix multiplication
in highp vec2 texcoord;
out highp vec2 vTexcoord;
void main()
{
    vTexcoord = texcoord;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
#version 300 es
precision mediump float;
uniform sampler2D texture;
in highp vec2 vTexcoord;
uniform float alpha;
out highp vec4 fragColor;
void main()
{
    fragColor = texture(texture, vTexcoord) * vec4(1.0, 1.0, 1.0, alpha > .0 ? alpha : 1.0);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoord");
    m_textureLocation = uniformLocation("texture");
}
