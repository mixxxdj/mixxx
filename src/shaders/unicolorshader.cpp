#include "shaders/unicolorshader.h"

using namespace mixxx;

void UnicolorShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute highp vec4 position;
void main()
{
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform highp vec4 color;
void main()
{
    gl_FragColor = color;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
