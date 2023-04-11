#include "waveform/renderers/qopengl/waveformrendererpreroll.h"

#include <QDomNode>

#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace qopengl;

WaveformRendererPreroll::WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidget)
        : WaveformShaderRenderer(waveformWidget) {
}

WaveformRendererPreroll::~WaveformRendererPreroll() {
}

void WaveformRendererPreroll::setup(
        const QDomNode& node, const SkinContext& context) {
    m_color.setNamedColor(context.selectString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::initializeGL() {
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

void WaveformRendererPreroll::renderGL() {
    const TrackPointer track = m_waveformRenderer->getTrackInfo();
    if (!track) {
        return;
    }

    m_vertices.clear();

    const double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    // Check if the pre- or post-roll is on screen. If so, draw little triangles
    // to indicate the respective zones.
    const bool preRollVisible = firstDisplayedPosition < 0;
    const bool postRollVisible = lastDisplayedPosition > 1;

    if (preRollVisible || postRollVisible) {
        const double playMarkerPosition = m_waveformRenderer->getPlayMarkerPosition();
        const double vSamplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
        const double numberOfVSamples = m_waveformRenderer->getLength() * vSamplesPerPixel;

        const int currentVSamplePosition = m_waveformRenderer->getPlayPosVSample();
        const int totalVSamples = m_waveformRenderer->getTotalVSample();

        const float halfBreadth = m_waveformRenderer->getBreadth() / 2.0f;
        const float halfPolyBreadth = m_waveformRenderer->getBreadth() / 5.0f;

        const double polyPixelWidth = 40.0 / vSamplesPerPixel;

        const int maxNumTriangles =
                static_cast<int>(
                        static_cast<double>(m_waveformRenderer->getLength()) /
                        polyPixelWidth) +
                1;
        const int numValuesPerTriangle = 6;
        // for most pessimistic case, where we fill the entire display with triangles
        m_vertices.reserve(maxNumTriangles * numValuesPerTriangle);

        if (preRollVisible) {
            // VSample position of the right-most triangle's tip
            const double triangleTipVSamplePosition =
                    numberOfVSamples * playMarkerPosition -
                    currentVSamplePosition;
            // In pixels
            double x = triangleTipVSamplePosition / vSamplesPerPixel;
            const double limit =
                    static_cast<double>(m_waveformRenderer->getLength()) +
                    polyPixelWidth;
            if (x >= limit) {
                // Don't draw invisible triangles beyond the right side of the display
                x -= std::floor((x - limit) / polyPixelWidth) * polyPixelWidth;
            }

            while (x >= 0) {
                const float x1 = static_cast<float>(x);
                const float x2 = static_cast<float>(x - polyPixelWidth);
                m_vertices.push_back(x1);
                m_vertices.push_back(halfBreadth);
                m_vertices.push_back(x2);
                m_vertices.push_back(halfBreadth - halfPolyBreadth);
                m_vertices.push_back(x2);
                m_vertices.push_back(halfBreadth + halfPolyBreadth);

                x -= polyPixelWidth;
            }
        }

        if (postRollVisible) {
            const int remainingVSamples = totalVSamples - currentVSamplePosition;
            // Sample position of the left-most triangle's tip
            const double triangleTipVSamplePosition =
                    playMarkerPosition * numberOfVSamples +
                    remainingVSamples;
            // In pixels
            double x = triangleTipVSamplePosition / vSamplesPerPixel;
            const double limit = -polyPixelWidth;
            if (x <= limit) {
                // Don't draw invisible triangles before the left side of the display
                x += std::floor((limit - x) / polyPixelWidth) * polyPixelWidth;
            }

            const double end = static_cast<double>(m_waveformRenderer->getLength());
            while (x < end) {
                const float x1 = static_cast<float>(x);
                const float x2 = static_cast<float>(x + polyPixelWidth);
                m_vertices.push_back(x1);
                m_vertices.push_back(halfBreadth);
                m_vertices.push_back(x2);
                m_vertices.push_back(halfBreadth - halfPolyBreadth);
                m_vertices.push_back(x2);
                m_vertices.push_back(halfBreadth + halfPolyBreadth);

                x += polyPixelWidth;
            }
        }
    }

    m_shaderProgram.bind();

    const int vertexLocation = m_shaderProgram.attributeLocation("position");
    const int matrixLocation = m_shaderProgram.uniformLocation("matrix");
    const int colorLocation = m_shaderProgram.uniformLocation("color");

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight()));
    if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
        matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
        matrix.translate(0.f, -m_waveformRenderer->getWidth(), 0.f);
    }

    m_shaderProgram.enableAttributeArray(vertexLocation);
    m_shaderProgram.setAttributeArray(
            vertexLocation, GL_FLOAT, m_vertices.constData(), 2);

    m_shaderProgram.setUniformValue(matrixLocation, matrix);
    m_shaderProgram.setUniformValue(colorLocation, m_color);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size() / 2);

    m_shaderProgram.disableAttributeArray(vertexLocation);
    m_shaderProgram.release();
}
