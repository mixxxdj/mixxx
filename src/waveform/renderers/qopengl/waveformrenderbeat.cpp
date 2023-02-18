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
    m_beatLineVertices.resize(1024);
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

    // TODO @m0dB use rendererWidth for vertical orientation
    // and apply a 90 degrees rotation to the matrix
    // const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    int vertexCount = 0;

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatPosition);

        xBeatPoint = qRound(xBeatPoint);

        // If we don't have enough space, double the size.
        if (vertexCount >= m_beatLineVertices.size()) {
            m_beatLineVertices.resize(m_beatLineVertices.size() * 2);
        }

        m_beatLineVertices[vertexCount++] = xBeatPoint;
        m_beatLineVertices[vertexCount++] = 0.f;
        m_beatLineVertices[vertexCount++] = xBeatPoint + 1;
        m_beatLineVertices[vertexCount++] = 0.f;
        m_beatLineVertices[vertexCount++] = xBeatPoint;
        m_beatLineVertices[vertexCount++] = rendererHeight;
        m_beatLineVertices[vertexCount++] = xBeatPoint;
        m_beatLineVertices[vertexCount++] = rendererHeight;
        m_beatLineVertices[vertexCount++] = xBeatPoint + 1;
        m_beatLineVertices[vertexCount++] = rendererHeight;
        m_beatLineVertices[vertexCount++] = xBeatPoint + 1;
        m_beatLineVertices[vertexCount++] = 0.f;
    }
    m_shaderProgram.bind();

    int vertexLocation = m_shaderProgram.attributeLocation("position");
    int matrixLocation = m_shaderProgram.uniformLocation("matrix");
    int colorLocation = m_shaderProgram.uniformLocation("color");

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight()));

    m_shaderProgram.enableAttributeArray(vertexLocation);
    m_shaderProgram.setAttributeArray(
            vertexLocation, GL_FLOAT, m_beatLineVertices.constData(), 2);

    m_shaderProgram.setUniformValue(matrixLocation, matrix);
    m_shaderProgram.setUniformValue(colorLocation, m_beatColor);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount / 2);
}
