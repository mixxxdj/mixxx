#include "waveform/renderers/qopengl/shaders/unicolorshader.h"

using namespace qopengl;

void UnicolorShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
void main()
{
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform vec4 color;
void main()
{
    gl_FragColor = color;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
