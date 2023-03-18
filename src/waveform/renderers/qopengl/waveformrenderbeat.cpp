#include "waveform/renderers/qopengl/waveformrenderbeat.h"

#include <QDomNode>

#include "control/controlobject.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/widgets/qopengl/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace qopengl;

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget)
        : WaveformShaderRenderer(waveformWidget) {
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::initializeGL() {
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

    initShaders(vertexShaderCode, fragmentShaderCode);
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();
}

void WaveformRenderBeat::renderGL() {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo) {
        return;
    }

    mixxx::BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats) {
        return;
    }

    int alpha = m_waveformRenderer->getBeatGridAlpha();
    if (alpha == 0) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_beatColor.setAlphaF(alpha / 100.0);

    const int trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    const auto startPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            firstDisplayedPosition * trackSamples);
    const auto endPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            lastDisplayedPosition * trackSamples);

    const float rendererBreadth = m_waveformRenderer->getBreadth();

    const int numVerticesPerLine = 12; // 2 vertices per point, 2 points per
                                       // line, 3 per triangle (2 triangles)
    const int reserved =
            trackBeats->numBeatsInRange(startPosition, endPosition) *
            numVerticesPerLine;

    m_beatLineVertices.clear();
    m_beatLineVertices.reserve(reserved);

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatPosition);

        xBeatPoint = std::floor(xBeatPoint);

        xBeatPoint -= 0.5;

        const float x1 = static_cast<float>(xBeatPoint);
        const float x2 = x1 + 1.f;

        m_beatLineVertices.push_back(x1);
        m_beatLineVertices.push_back(0.f);
        m_beatLineVertices.push_back(x2);
        m_beatLineVertices.push_back(0.f);
        m_beatLineVertices.push_back(x1);
        m_beatLineVertices.push_back(rendererBreadth);
        m_beatLineVertices.push_back(x1);
        m_beatLineVertices.push_back(rendererBreadth);
        m_beatLineVertices.push_back(x2);
        m_beatLineVertices.push_back(rendererBreadth);
        m_beatLineVertices.push_back(x2);
        m_beatLineVertices.push_back(0.f);
    }

    DEBUG_ASSERT(reserved == m_beatLineVertices.size());

    m_shaderProgram.bind();

    int vertexLocation = m_shaderProgram.attributeLocation("position");
    int matrixLocation = m_shaderProgram.uniformLocation("matrix");
    int colorLocation = m_shaderProgram.uniformLocation("color");

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0.0, 0.0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight()));
    if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
        matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
        matrix.translate(0.f, -m_waveformRenderer->getWidth(), 0.f);
    }

    m_shaderProgram.enableAttributeArray(vertexLocation);
    m_shaderProgram.setAttributeArray(
            vertexLocation, GL_FLOAT, m_beatLineVertices.constData(), 2);

    m_shaderProgram.setUniformValue(matrixLocation, matrix);
    m_shaderProgram.setUniformValue(colorLocation, m_beatColor);

    glDrawArrays(GL_TRIANGLES, 0, m_beatLineVertices.size() / 2);
}
