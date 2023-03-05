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
    m_vertices.resize(1024);
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

    double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    // Check if the pre- or post-roll is on screen. If so, draw little triangles
    // to indicate the respective zones.
    bool preRollVisible = firstDisplayedPosition < 0;
    bool postRollVisible = lastDisplayedPosition > 1;
    int vertexCount = 0;
    if (preRollVisible || postRollVisible) {
        const double playMarkerPositionFrac = m_waveformRenderer->getPlayMarkerPosition();
        const double vSamplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
        const double numberOfVSamples = m_waveformRenderer->getLength() * vSamplesPerPixel;

        const int currentVSamplePosition = m_waveformRenderer->getPlayPosVSample();
        const int totalVSamples = m_waveformRenderer->getTotalVSample();
        // qDebug() << "currentVSamplePosition" << currentVSamplePosition
        //          << "lastDisplayedPosition" << lastDisplayedPosition
        //          << "vSamplesPerPixel" << vSamplesPerPixel
        //          << "numberOfVSamples" << numberOfVSamples
        //          << "totalVSamples" << totalVSamples
        //          << "WaveformRendererPreroll::playMarkerPosition=" << playMarkerPositionFrac;

        const float halfBreadth = m_waveformRenderer->getBreadth() / 2.0f;
        const float halfPolyBreadth = m_waveformRenderer->getBreadth() / 5.0f;

        /*
        PainterScope PainterScope(painter);

        painter->setRenderHint(QPainter::Antialiasing);
        //painter->setRenderHint(QPainter::HighQualityAntialiasing);
        //painter->setBackgroundMode(Qt::TransparentMode);
        painter->setWorldMatrixEnabled(false);
        painter->setPen(QPen(QBrush(m_color), std::max(1.0, scaleFactor())));
        */

        const double polyPixelWidth = 40.0 / vSamplesPerPixel;
        const double polyPixelOffset = polyPixelWidth; // TODO @m0dB + painter->pen().widthF();
        const double polyVSampleOffset = polyPixelOffset * vSamplesPerPixel;

        if (preRollVisible) {
            // VSample position of the right-most triangle's tip
            double triangleTipVSamplePosition =
                    numberOfVSamples * playMarkerPositionFrac -
                    currentVSamplePosition;

            double x = triangleTipVSamplePosition / vSamplesPerPixel;

            for (; triangleTipVSamplePosition > 0;
                    triangleTipVSamplePosition -= polyVSampleOffset) {
                const float x1 = static_cast<float>(x);
                const float x2 = static_cast<float>(x - polyPixelWidth);
                m_vertices[vertexCount++] = x1;
                m_vertices[vertexCount++] = halfBreadth;
                m_vertices[vertexCount++] = x2;
                m_vertices[vertexCount++] = halfBreadth - halfPolyBreadth;
                m_vertices[vertexCount++] = x2;
                m_vertices[vertexCount++] = halfBreadth + halfPolyBreadth;

                x -= polyPixelOffset;
            }
        }

        if (postRollVisible) {
            const int remainingVSamples = totalVSamples - currentVSamplePosition;
            // Sample position of the left-most triangle's tip
            double triangleTipVSamplePosition =
                    playMarkerPositionFrac * numberOfVSamples +
                    remainingVSamples;

            double x = triangleTipVSamplePosition / vSamplesPerPixel;

            for (; triangleTipVSamplePosition < numberOfVSamples;
                    triangleTipVSamplePosition += polyVSampleOffset) {
                const float x1 = static_cast<float>(x);
                const float x2 = static_cast<float>(x + polyPixelWidth);
                m_vertices[vertexCount++] = x1;
                m_vertices[vertexCount++] = halfBreadth;
                m_vertices[vertexCount++] = x2;
                m_vertices[vertexCount++] = halfBreadth - halfPolyBreadth;
                m_vertices[vertexCount++] = x2;
                m_vertices[vertexCount++] = halfBreadth + halfPolyBreadth;

                x += polyPixelOffset;
            }
        }
    }

    m_shaderProgram.bind();

    int vertexLocation = m_shaderProgram.attributeLocation("position");
    int matrixLocation = m_shaderProgram.uniformLocation("matrix");
    int colorLocation = m_shaderProgram.uniformLocation("color");

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

    glDrawArrays(GL_TRIANGLES, 0, vertexCount / 2);
}
