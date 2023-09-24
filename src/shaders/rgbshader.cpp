#include "shaders/rgbshader.h"

using namespace mixxx;

void RGBShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute highp vec4 position;
attribute highp vec3 color;
varying highp vec3 vcolor;
void main()
{
    vcolor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
varying highp vec3 vcolor;
void main()
{
    gl_FragColor = vec4(vcolor,1.0);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
