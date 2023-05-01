#include "waveform/renderers/allshader/waveformrendererpreroll.h"

#include <QDomNode>

#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace allshader;

WaveformRendererPreroll::WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget) {
}

WaveformRendererPreroll::~WaveformRendererPreroll() {
}

void WaveformRendererPreroll::setup(
        const QDomNode& node, const SkinContext& context) {
    m_color.setNamedColor(context.selectString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::initializeGL() {
    m_shader.init();
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

    if (!(preRollVisible || postRollVisible)) {
        return;
    }

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
            2;
    const int numVerticesPerTriangle = 3;
    const int reserved = maxNumTriangles * numVerticesPerTriangle;
    // for most pessimistic case, where we fill the entire display with triangles
    m_vertices.reserve(reserved);

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
            x -= std::ceil((x - limit) / polyPixelWidth) * polyPixelWidth;
        }

        while (x >= 0) {
            const float x1 = static_cast<float>(x);
            const float x2 = static_cast<float>(x - polyPixelWidth);
            m_vertices.addTriangle({x1, halfBreadth},
                    {x2, halfBreadth - halfPolyBreadth},
                    {x2, halfBreadth + halfPolyBreadth});

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
            x += std::ceil((limit - x) / polyPixelWidth) * polyPixelWidth;
        }

        const double end = static_cast<double>(m_waveformRenderer->getLength());
        while (x < end) {
            const float x1 = static_cast<float>(x);
            const float x2 = static_cast<float>(x + polyPixelWidth);
            m_vertices.addTriangle({x1, halfBreadth},
                    {x2, halfBreadth - halfPolyBreadth},
                    {x2, halfBreadth + halfPolyBreadth});
            x += polyPixelWidth;
        }
    }

    DEBUG_ASSERT(m_vertices.size() <= reserved);

    const int vertexLocation = m_shader.attributeLocation("position");
    const int matrixLocation = m_shader.uniformLocation("matrix");
    const int colorLocation = m_shader.uniformLocation("color");

    m_shader.bind();
    m_shader.enableAttributeArray(vertexLocation);

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    m_shader.setAttributeArray(
            vertexLocation, GL_FLOAT, m_vertices.constData(), 2);

    m_shader.setUniformValue(matrixLocation, matrix);
    m_shader.setUniformValue(colorLocation, m_color);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_shader.disableAttributeArray(vertexLocation);
    m_shader.release();
}
