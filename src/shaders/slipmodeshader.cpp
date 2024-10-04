#include "shaders/slipmodeshader.h"

using namespace mixxx;

void SlipModeShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
attribute highp vec4 position; // use vec4 here (will be padded) to assign directly to gl_Position

varying highp vec4 vposition;

void main()
{
    vposition = position;
    gl_Position = position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform highp vec4 color;
uniform highp vec2 borders;
uniform highp vec2 dimension;

varying highp vec4 vposition;

void main()
{

    highp float xBorder = abs(dimension.x * vposition.x);
    highp float yBorder = dimension.y * vposition.y;
    highp float upperBoard = borders.x;
    highp float lowerBoard = borders.y;

    if (yBorder < 0.0){
        highp float borderAlpha = max(
            0.0, 
            lowerBoard + yBorder
        ) / lowerBoard;
        gl_FragColor = vec4(color.xyz, mix(0.0, color.w, borderAlpha));

    } else if( (xBorder > dimension.x - upperBoard && yBorder >= 0.0) || yBorder > dimension.y - upperBoard)
    {
        highp float borderAlpha = max(0.0, max(
            yBorder - dimension.y, 
            xBorder - dimension.x) + upperBoard) / upperBoard;
        gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb, min(color.w, borderAlpha)), 1.0);
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_positionLocation = attributeLocation("position");
    m_boarderLocation = uniformLocation("borders");
    m_dimensionLocation = uniformLocation("dimension");
    m_colorLocation = uniformLocation("color");
}
