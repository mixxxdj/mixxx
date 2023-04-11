#include "waveform/renderers/qopengl/shaders/gradientshader.h"

using namespace qopengl;

void GradientShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 gradient;
varying vec3 vGradient;
void main()
{
    vGradient = gradient;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform vec4 color;
varying vec3 vGradient;
void main()
{
    gl_FragColor = vec4(color.x, color.y, color.z, color.w * max(0.0, abs((vGradient.x + vGradient.y) * 4.0 - 2.0) - 1.0));
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
