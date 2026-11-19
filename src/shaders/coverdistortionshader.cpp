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

    // The foreground is the record. Everything is calculated in a coordinate
    // system with the origin in the center of the record and its diameter as
    // unit. First the position of the pixel is moved by the distortions, each
    // of them scaled by its amount, in this order:
    // bass: the center bulges towards the viewer and a ring runs outwards.
    // highs: the picture glitches like on an old television: thin horizontal
    //        strips are torn sideways and lose their colors, a wobbling bar
    //        runs down the screen, and noise covers the picture. The strips
    //        change several times per second. Towards its lower edge a strip
    //        slides over into the next one, which smears the picture there
    //        instead of cutting it.
    // mids: fine ripples shimmer and the red and blue channels drift apart.
    // The disc, its hole and its grooves are derived from the moved position,
    // so the record is distorted as a whole. Only the lookup of the picture
    // uses the position turned by the spin. The grooves are never finer than a
    // few pixels, otherwise they would flicker. The reflections depend on the
    // direction that is not turned, as the light does not turn with the record.
    // Where the record does not reach, the background shows. It is the dimmed
    // blurred copy of the picture, displaced by two layers of slow waves that
    // complete a cycle every one and every half period of the flow phase.
    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D texture;
uniform sampler2D blurredTexture;
uniform highp vec2 fit;
uniform highp vec2 backgroundFit;
uniform highp vec2 crop;
uniform highp float windowAspect;
uniform highp vec3 amounts;
uniform highp vec2 phases;
uniform highp float flowPhase;
uniform highp float spin;
uniform highp float pixelSize;
varying highp vec2 vTexcoord;

const highp float kHoleRadius = 0.022;
const highp float kLabelRadius = 0.17;
const highp float kGrooveStart = 0.182;
const highp float kGrooveEnd = 0.484;
const highp float kDiscRadius = 0.5;
const highp float kFinestGroove = 1.0 / 240.0;
const highp vec2 kLightDirection = vec2(0.7071, -0.7071);

highp vec2 mirrored(highp vec2 uv)
{
    highp vec2 m = mod(uv, 2.0);
    return mix(m, 2.0 - m, step(1.0, m));
}

highp vec2 turned(highp vec2 centered)
{
    highp float s = sin(spin);
    highp float co = cos(spin);
    return vec2(centered.x * co + centered.y * s, centered.y * co - centered.x * s);
}

highp vec2 pictureCoord(highp vec2 centered)
{
    return centered * crop + 0.5;
}

highp vec3 color(highp vec4 texel)
{
    return texel.rgb / max(texel.a, 0.001);
}

highp vec3 background()
{
    highp vec2 q = (vTexcoord - 0.5) * vec2(windowAspect, 1.0);
    highp vec2 flow = vec2(
            sin(q.y * 3.1 + flowPhase) + 0.5 * sin(q.y * 5.3 + q.x * 2.0 - 2.0 * flowPhase),
            cos(q.x * 2.7 - flowPhase) + 0.5 * cos(q.x * 4.6 - q.y * 2.3 + 2.0 * flowPhase));
    q += 0.08 * flow;
    highp vec2 uv = q / vec2(windowAspect, 1.0) * backgroundFit + 0.5;
    return color(texture2D(blurredTexture, mirrored(uv))) * 0.6;
}

highp float random(highp vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

// Returns whether a strip of a glitch is torn, and how far.
highp vec2 stripTear(highp float strip, highp float glitch, highp float amount)
{
    highp float torn = step(0.85 - 0.5 * amount, random(vec2(strip, glitch)));
    return vec2(torn, torn * 0.28 * (random(vec2(strip, glitch + 0.5)) - 0.5));
}

highp float lineTear(highp float line, highp float glitch)
{
    return step(0.8, random(vec2(line, glitch))) * 0.05 *
            (random(vec2(line, glitch + 0.5)) - 0.5);
}

highp float ring(highp float radius, highp float center, highp float width)
{
    return 1.0 - smoothstep(0.5 * width, width, abs(radius - center));
}

void main()
{
    highp float bass = amounts.x;
    highp float mid = amounts.y;
    highp float high = amounts.z;

    highp vec2 c = (vTexcoord - 0.5) * fit;
    highp float r = length(c);

    c *= 1.0 - bass * 0.28 * exp(-r * r * 3.0);
    c += c * bass * 0.09 * sin(r * 16.0 - 3.0 * phases.y);

    // The fast phase is divided into 12 steps and the slow one into 16.
    highp float glitch = floor(phases.y * 1.9099) + 13.0 * floor(phases.x * 2.5465);
    highp float strip = c.y * (60.0 + 60.0 * random(vec2(glitch, 1.0))) +
            7.0 * random(vec2(glitch, 2.0));
    highp float line = c.y * 240.0;
    highp vec2 stripTorn = mix(stripTear(floor(strip), glitch, high),
            stripTear(floor(strip) + 1.0, glitch, high),
            smoothstep(0.3, 1.0, fract(strip)));
    highp float torn = stripTorn.x;
    highp float tear = stripTorn.y +
            mix(lineTear(floor(line), glitch),
                    lineTear(floor(line) + 1.0, glitch),
                    smoothstep(0.0, 1.0, fract(line)));
    // The bar runs down the screen three times per cycle of the slow phase.
    highp float bar = 1.0 - smoothstep(0.0, 0.06,
            abs(c.y - 1.4 * (fract(phases.x * 0.47746) - 0.5)));
    highp float grain = random(floor((vTexcoord - 0.5) * fit / (2.0 * pixelSize)) + glitch) - 0.5;
    c.x += high * (tear + 0.05 * bar * sin(c.y * 90.0 + 5.0 * phases.y));
    highp vec2 split = vec2(high * (0.006 + 0.02 * torn), 0.0);

    c += mid * 0.01 * vec2(sin(c.y * 90.0 + 7.0 * phases.y), cos(c.x * 80.0 + 5.0 * phases.y));
    highp float fringe = mid * 0.035;

    highp float radius = length(c);

    highp vec3 record = vec3(
            color(texture2D(texture, pictureCoord(turned(c + split) * (1.0 + fringe)))).r,
            color(texture2D(texture, pictureCoord(turned(c)))).g,
            color(texture2D(texture, pictureCoord(turned(c - split) * (1.0 - fringe)))).b);

    highp float grooved = smoothstep(kLabelRadius, kGrooveStart, radius) *
            (1.0 - smoothstep(kGrooveEnd, kGrooveEnd + 0.006, radius));
    highp float gapWidth = max(0.005, 2.5 * pixelSize);
    highp float gaps = max(max(ring(radius, 0.262, gapWidth), ring(radius, 0.349, gapWidth)),
            ring(radius, 0.421, gapWidth));
    highp float groovePeriod = max(kFinestGroove, 5.0 * pixelSize);
    highp float groove = 0.5 + 0.5 * sin(6.2832 * radius / groovePeriod);
    highp float depth = grooved * (1.0 - gaps) * (0.8 + 0.2 * sin(radius * 61.0));

    highp float facing = abs(dot(c, kLightDirection)) / max(radius, 0.0001);
    highp float reflection = pow(facing, 18.0) * grooved;

    record *= 1.0 - 0.1 * grooved;
    record *= 1.0 - 0.24 * depth * groove;
    record *= 1.0 - 0.35 * smoothstep(kGrooveEnd, kDiscRadius, radius);
    record *= 1.0 - 0.3 * ring(radius, 0.5 * (kLabelRadius + kGrooveStart), 0.008);
    record += reflection * (0.1 + 0.16 * depth * (1.0 - groove));
    record += high * (0.12 * grain * (0.4 + 0.6 * torn) + 0.1 * bar);

    highp float disc = clamp((kDiscRadius - radius) / pixelSize + 0.5, 0.0, 1.0) *
            clamp((radius - kHoleRadius) / pixelSize + 0.5, 0.0, 1.0);

    gl_FragColor = vec4(mix(background(), record, disc), 1.0);
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
    m_cropLocation = uniformLocation("crop");
    m_spinLocation = uniformLocation("spin");
    m_pixelSizeLocation = uniformLocation("pixelSize");
    m_amountsLocation = uniformLocation("amounts");
    m_phasesLocation = uniformLocation("phases");
}
