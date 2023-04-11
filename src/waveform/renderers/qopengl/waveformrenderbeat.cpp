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
        : WaveformRenderer(waveformWidget) {
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::initializeGL() {
    m_shader.init();
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_color.setNamedColor(context.selectString(node, "BeatColor"));
    m_color = WSkinColor::getCorrectColor(m_color).toRgb();
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

    m_color.setAlphaF(alpha / 100.0);

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

    const int numVerticesPerLine = 6; // 2 triangles
    const int numBeatsInRange = trackBeats->numBeatsInRange(startPosition, endPosition);

    // In corner cases numBeatsInRange returns -1, while the for loop below
    // iterates 1 time, resulting in a mismatch between reserved and drawn
    // lines. This probably should be fixed in beats.h/cpp but for now just
    // return.
    if (numBeatsInRange <= 0) {
        return;
    }

    const int reserved = numBeatsInRange * numVerticesPerLine;
    m_vertices.clear();
    m_vertices.reserve(reserved);

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

        m_vertices.push_back({x1, 0.f});
        m_vertices.push_back({x2, 0.f});
        m_vertices.push_back({x1, rendererBreadth});
        m_vertices.push_back({x1, rendererBreadth});
        m_vertices.push_back({x2, rendererBreadth});
        m_vertices.push_back({x2, 0.f});
    }

    DEBUG_ASSERT(reserved == m_vertices.size());

    const int vertexLocation = m_shader.attributeLocation("position");
    const int matrixLocation = m_shader.uniformLocation("matrix");
    const int colorLocation = m_shader.uniformLocation("color");

    m_shader.bind();
    m_shader.enableAttributeArray(vertexLocation);

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0.0, 0.0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight()));
    if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
        matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
        matrix.translate(0.f, -m_waveformRenderer->getWidth(), 0.f);
    }

    m_shader.setAttributeArray(
            vertexLocation, GL_FLOAT, m_vertices.constData(), 2);

    m_shader.setUniformValue(matrixLocation, matrix);
    m_shader.setUniformValue(colorLocation, m_color);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_shader.disableAttributeArray(vertexLocation);
    m_shader.release();
}
