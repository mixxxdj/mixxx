#include "shaders/vinylqualityshader.h"

using namespace mixxx;

void VinylQualityShader::init() {
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
uniform vec4 color;
varying vec3 vTexcoor;
void main()
{
    gl_FragColor = vec4(color.xyz, texture2D(sampler, vTexcoor.xy) * 0.75);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_samplerLocation = uniformLocation("sampler");
    m_colorLocation = uniformLocation("color");
    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoor");
}
