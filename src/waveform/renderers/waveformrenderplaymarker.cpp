#include "waveformrenderplaymarker.h"

#include "util/frameadapter.h"

namespace {
mixxx::FramePos rendererPositionFractionToTrackPosition(
        double rendererPositionFraction, int trackSamples) {
    return samplePosToFramePos(rendererPositionFraction * trackSamples);
}
constexpr double kBarBeatTextBoxOpacity = 0.9;
} // namespace

WaveformRenderPlayMarker::WaveformRenderPlayMarker(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
}

WaveformRenderPlayMarker::~WaveformRenderPlayMarker() {
}

void WaveformRenderPlayMarker::setup(
        const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WaveformRenderPlayMarker::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(event);
    auto orientation = m_waveformRenderer->getOrientation();
    const int height = m_waveformRenderer->getHeight();
    const int width = m_waveformRenderer->getWidth();
    const double playMarkerPosition =
            m_waveformRenderer->getPlayMarkerPosition();
    const int lineX = width * playMarkerPosition;
    const int lineY = height * playMarkerPosition;
    const auto colors = m_waveformRenderer->getWaveformSignalColors();
    const int trackSamples = m_waveformRenderer->getTrackSamples();
    const auto trackBeats = m_waveformRenderer->getTrackInfo()->getBeats();
    const auto currentTrackPosition = rendererPositionFractionToTrackPosition(
            m_waveformRenderer->getPlayPos(), trackSamples);
    const auto prevBeat = trackBeats->findPrevBeat(currentTrackPosition);

    // Draw the play marker line
    painter->setPen(colors->getPlayPosColor());
    if (orientation == Qt::Horizontal) {
        painter->drawLine(lineX, 0, lineX, height);
    } else {
        painter->drawLine(0, lineY, width, lineY);
    }
    painter->setOpacity(0.5);
    painter->setPen(colors->getBgColor());
    if (orientation == Qt::Horizontal) {
        painter->drawLine(lineX + 1, 0, lineX + 1, height);
        painter->drawLine(lineX - 1, 0, lineX - 1, height);
    } else {
        painter->drawLine(0, lineY + 1, width, lineY + 1);
        painter->drawLine(0, lineY - 1, width, lineY - 1);
    }

    // Draw bar and beat counter
    if (prevBeat != mixxx::kInvalidBeat) {
        QString barBeatString = QString("%1.%2").arg(
                QString::number(prevBeat.getBarIndex() + 1),
                QString::number(prevBeat.getBarRelativeBeatIndex() + 1));
        painter->setPen(colors->getPlayPosColor());
        painter->setBrush(colors->getBgColor());
        painter->setOpacity(kBarBeatTextBoxOpacity);
        const int boxHeightPixels = painter->fontMetrics().height();
        const int boxWidthPixels = painter->fontMetrics().width(
                barBeatString, barBeatString.length());
        const int textPaddingPixels = 2;
        if (orientation == Qt::Horizontal) {
            painter->drawRect(lineX,
                    height / 2 - boxHeightPixels / 2 - textPaddingPixels,
                    boxWidthPixels + 2 * textPaddingPixels,
                    boxHeightPixels + 2 * textPaddingPixels);
            painter->drawText(lineX + textPaddingPixels,
                    height / 2 + boxHeightPixels / 2 - textPaddingPixels,
                    barBeatString);
        } else {
            painter->drawRect(
                    width / 2 - boxWidthPixels / 2 - textPaddingPixels,
                    lineY - boxHeightPixels - textPaddingPixels * 2,
                    boxWidthPixels + 2 * textPaddingPixels,
                    boxHeightPixels + 2 * textPaddingPixels);
            painter->drawText(width / 2 - boxWidthPixels / 2,
                    lineY - textPaddingPixels,
                    barBeatString);
        }
    }
}
