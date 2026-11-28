#include "shaders/covercombineshader.h"

using namespace mixxx;

void CoverCombineShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
attribute highp vec4 position;
attribute highp vec2 texcoord;
varying highp vec2 vTexcoord;
void main()
{
    vTexcoord = texcoord;
    gl_Position = position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D texture;
uniform highp vec3 borders[3];
uniform highp float sharpness;
uniform highp vec2 fit;
varying highp vec2 vTexcoord;

void main()
{
    highp vec2 position = vTexcoord - 0.5;
    highp float coverage = 1.0;
    for (int i = 0; i < 3; ++i) {
        highp float distance = dot(borders[i].xy, position) + borders[i].z;
        coverage *= clamp(distance * sharpness + 0.5, 0.0, 1.0);
    }
    if (coverage <= 0.0) {
        discard;
    }
    gl_FragColor = vec4(texture2D(texture, position * fit + 0.5).rgb, 1.0) * coverage;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoord");
    m_textureLocation = uniformLocation("texture");
    m_bordersLocation = uniformLocation("borders");
    m_sharpnessLocation = uniformLocation("sharpness");
    m_fitLocation = uniformLocation("fit");
}
