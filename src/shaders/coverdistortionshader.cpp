#include "shaders/coverdistortionshader.h"

using namespace mixxx;

void CoverDistortionShader::init() {
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

    // [AI-generated] The foreground is the cover. All of its distortions are
    // calculated in a coordinate system with the origin in the center of the
    // cover and the height of the cover as unit, and each of them is scaled
    // by its amount. They are applied in this order:
    // bass: the center bulges towards the viewer and a ring runs outwards.
    // mids: the image swirls around the center and flows like a liquid.
    // highs: fine ripples shimmer and the red and blue channels drift apart.
    // Where the distorted cover does not reach, the background shows. It is
    // the dimmed blurred copy of the cover, displaced by two layers of slow
    // waves that complete a cycle every one and every half period of the
    // flow phase. [/AI-generated]
    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D texture;
uniform sampler2D blurredTexture;
uniform highp vec2 fit;
uniform highp vec2 backgroundFit;
uniform highp float coverAspect;
uniform highp float windowAspect;
uniform highp vec3 amounts;
uniform highp vec2 phases;
uniform highp float flowPhase;
uniform highp float alpha;
varying highp vec2 vTexcoord;

highp vec2 mirrored(highp vec2 uv)
{
    highp vec2 m = mod(uv, 2.0);
    return mix(m, 2.0 - m, step(1.0, m));
}

highp vec2 coverCoord(highp vec2 centered)
{
    return centered / vec2(coverAspect, 1.0) + 0.5;
}

highp vec3 background()
{
    highp vec2 q = (vTexcoord - 0.5) * vec2(windowAspect, 1.0);
    highp vec2 flow = vec2(
            sin(q.y * 3.1 + flowPhase) + 0.5 * sin(q.y * 5.3 + q.x * 2.0 - 2.0 * flowPhase),
            cos(q.x * 2.7 - flowPhase) + 0.5 * cos(q.x * 4.6 - q.y * 2.3 + 2.0 * flowPhase));
    q += 0.08 * flow;
    highp vec2 uv = q / vec2(windowAspect, 1.0) * backgroundFit + 0.5;
    return texture2D(blurredTexture, mirrored(uv)).rgb * 0.6;
}

void main()
{
    highp float bass = amounts.x;
    highp float mid = amounts.y;
    highp float high = amounts.z;

    highp vec2 c = (vTexcoord - 0.5) * fit * vec2(coverAspect, 1.0);
    highp float r = length(c);

    c *= 1.0 - bass * 0.28 * exp(-r * r * 3.0);
    c += c * bass * 0.09 * sin(r * 16.0 - 3.0 * phases.y);

    highp float angle = mid * 0.9 * exp(-r * 1.6) * sin(phases.x + r * 3.0);
    highp float s = sin(angle);
    highp float co = cos(angle);
    c = vec2(c.x * co - c.y * s, c.x * s + c.y * co);
    c.x += mid * 0.045 * sin(c.y * 7.0 + 2.0 * phases.x);
    c.y += mid * 0.045 * cos(c.x * 6.0 + 3.0 * phases.x);

    c += high * 0.01 * vec2(sin(c.y * 90.0 + 7.0 * phases.y), cos(c.x * 80.0 + 5.0 * phases.y));
    highp float fringe = high * 0.035;

    highp vec2 uv = coverCoord(c);
    highp vec3 foreground = vec3(
            texture2D(texture, coverCoord(c * (1.0 + fringe))).r,
            texture2D(texture, uv).g,
            texture2D(texture, coverCoord(c * (1.0 - fringe))).b);

    highp vec2 feather = vec2(0.004);
    highp vec2 inside = smoothstep(vec2(0.0), feather, uv) *
            (1.0 - smoothstep(1.0 - feather, vec2(1.0), uv));

    gl_FragColor = vec4(mix(background(), foreground, inside.x * inside.y), 1.0) * alpha;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_positionLocation = attributeLocation("position");
    m_texcoordLocation = attributeLocation("texcoord");
    m_textureLocation = uniformLocation("texture");
    m_blurredTextureLocation = uniformLocation("blurredTexture");
    m_fitLocation = uniformLocation("fit");
    m_backgroundFitLocation = uniformLocation("backgroundFit");
    m_windowAspectLocation = uniformLocation("windowAspect");
    m_flowPhaseLocation = uniformLocation("flowPhase");
    m_coverAspectLocation = uniformLocation("coverAspect");
    m_amountsLocation = uniformLocation("amounts");
    m_phasesLocation = uniformLocation("phases");
    m_alphaLocation = uniformLocation("alpha");
}
