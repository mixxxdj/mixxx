#include "waveform/renderers/allshader/waveformrendererfiltered.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

WaveformRendererFiltered::WaveformRendererFiltered(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererFiltered::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererFiltered::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
    m_vertices.create();
    m_vertices.setUsagePattern(QOpenGLBuffer::DynamicDraw);
}

void WaveformRendererFiltered::paintGL() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == nullptr) {
        return;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition() * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition() * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain{1.0};
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, true, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x
    double xVisualFrame = firstVisualFrame;

    const int numVerticesPerLine = 6; // 2 triangles

    int startIndex[4];
    int endIndex[4];
    int reserved[4];
    int reservedTotal = 0;
    // low, mid, high
    for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
        startIndex[bandIndex] = reservedTotal;
        endIndex[bandIndex] = reservedTotal;
        reserved[bandIndex] = numVerticesPerLine * length;
        reservedTotal += reserved[bandIndex];
        qDebug() << startIndex[bandIndex] << endIndex[bandIndex]
                 << reserved[bandIndex] << reservedTotal;
    }

    // the horizontal line
    startIndex[3] = reservedTotal;
    endIndex[3] = reservedTotal;
    reserved[3] = numVerticesPerLine;
    reservedTotal += reserved[3];

    m_vertices.bind();
    m_vertices.reserve(reservedTotal);
    m_vertices.mapForWrite();

    m_vertices.setIndex(endIndex[3]);
    m_vertices.addRectangle(
            0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            halfBreadth + 0.5f * devicePixelRatio);
    endIndex[3] += numVerticesPerLine;

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int pos = 0; pos < length; ++pos) {
        const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
        const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

        const int visualIndexStart = std::max(visualFrameStart * 2, 0);
        const int visualIndexStop =
                std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

        const float fpos = static_cast<float>(pos);

        // 3 bands, 2 channels
        float max[3][2]{};

        for (int i = visualIndexStart; i < visualIndexStop; i += 2) {
            for (int chn = 0; chn < 2; chn++) {
                const WaveformData& waveformData = data[i + chn];
                const float filteredLow = static_cast<float>(waveformData.filtered.low);
                const float filteredMid = static_cast<float>(waveformData.filtered.mid);
                const float filteredHigh = static_cast<float>(waveformData.filtered.high);

                max[0][chn] = math_max(max[0][chn], filteredLow);
                max[1][chn] = math_max(max[1][chn], filteredMid);
                max[2][chn] = math_max(max[2][chn], filteredHigh);
            }
        }

        for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
            max[bandIndex][0] *= bandGain[bandIndex];
            max[bandIndex][1] *= bandGain[bandIndex];

            // lines are thin rectangles
            m_vertices.setIndex(endIndex[bandIndex]);
            m_vertices.addRectangle(
                    fpos - 0.5f,
                    halfBreadth - heightFactor * max[bandIndex][0],
                    fpos + 0.5f,
                    halfBreadth + heightFactor * max[bandIndex][1]);
            endIndex[bandIndex] += numVerticesPerLine;
        }

        xVisualFrame += visualIncrementPerPixel;
    }

    m_vertices.unmap();

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    const int matrixLocation = m_shader.matrixLocation();
    const int colorLocation = m_shader.colorLocation();
    const int positionLocation = m_shader.positionLocation();

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);

    m_shader.setUniformValue(matrixLocation, matrix);

    QColor colors[4];
    colors[0].setRgbF(static_cast<float>(m_rgbLowColor_r),
            static_cast<float>(m_rgbLowColor_g),
            static_cast<float>(m_rgbLowColor_b));
    colors[1].setRgbF(static_cast<float>(m_rgbMidColor_r),
            static_cast<float>(m_rgbMidColor_g),
            static_cast<float>(m_rgbMidColor_b));
    colors[2].setRgbF(static_cast<float>(m_rgbHighColor_r),
            static_cast<float>(m_rgbHighColor_g),
            static_cast<float>(m_rgbHighColor_b));
    colors[3].setRgbF(static_cast<float>(m_axesColor_r),
            static_cast<float>(m_axesColor_g),
            static_cast<float>(m_axesColor_b),
            static_cast<float>(m_axesColor_a));

    m_shader.setAttributeBuffer(positionLocation,
            GL_FLOAT,
            m_vertices.offset(),
            m_vertices.tupleSize(),
            m_vertices.stride());

    // 3 bands + 1 extra for the horizontal line
    for (int bandIndex = 0; bandIndex < 4; bandIndex++) {
        DEBUG_ASSERT(reserved[bandIndex] == endIndex[bandIndex] - startIndex[bandIndex]);
        m_shader.setUniformValue(colorLocation, colors[bandIndex]);

        glDrawArrays(GL_TRIANGLES,
                startIndex[bandIndex],
                endIndex[bandIndex] - startIndex[bandIndex]);
    }
    DEBUG_ASSERT(reservedTotal == endIndex[3]);

    m_shader.disableAttributeArray(positionLocation);
    m_vertices.release();
    m_shader.release();
}

} // namespace allshader
