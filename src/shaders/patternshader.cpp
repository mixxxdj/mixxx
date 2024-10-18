#include "shaders/patternshader.h"

using namespace mixxx;

void PatternShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform highp mat4 matrix;
attribute highp vec4 position; // use vec4 here (will be padded) for matrix multiplication
attribute highp vec2 texcoor;
varying highp vec2 vTexcoor;
void main()
{
    vTexcoor = texcoor;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D texture;
uniform highp vec2 repetitions;
varying highp vec2 vTexcoor;
void main()
{
    gl_FragColor = texture2D(texture, fract(vTexcoor * repetitions));
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoor");
    m_textureLocation = uniformLocation("texture");
    m_repetitionsLocation = uniformLocation("repetitions");
}
