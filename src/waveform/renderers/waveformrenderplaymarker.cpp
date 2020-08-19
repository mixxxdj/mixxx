#include "waveformrenderplaymarker.h"

#include "util/frameadapter.h"

namespace {
mixxx::FramePos rendererPositionFractionToTrackPosition(
        double rendererPositionFraction, int trackSamples) {
    return samplePosToFramePos(rendererPositionFraction * trackSamples);
}
constexpr double kBarBeatTextBoxOpacity = 0.9;
const QString kBarBeatSeparator = ".";
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
    if (!trackBeats) {
        return;
    }
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
        QString barBeatString = generateBarBeatDisplayTextFromPrevBeat(prevBeat);
        painter->setPen(colors->getPlayPosColor());
        painter->setBrush(colors->getBgColor());
        painter->setOpacity(kBarBeatTextBoxOpacity);
        const int boxHeightPixels = painter->fontMetrics().height();
        const int boxWidthPixels = painter->fontMetrics().width(
                barBeatString, barBeatString.length());
        const int textPaddingPixels = 2;
        const int textDisplayBoxInitialShiftPixels = 1;
        if (orientation == Qt::Horizontal) {
            painter->drawRect(lineX,
                    textDisplayBoxInitialShiftPixels,
                    boxWidthPixels + 2 * textPaddingPixels,
                    boxHeightPixels + 2 * textPaddingPixels);
            painter->drawText(lineX + textPaddingPixels,
                    boxHeightPixels,
                    barBeatString);
        } else {
            painter->drawRect(
                    textDisplayBoxInitialShiftPixels,
                    lineY - boxHeightPixels - textPaddingPixels * 2,
                    boxWidthPixels + 2 * textPaddingPixels,
                    boxHeightPixels + 2 * textPaddingPixels);
            painter->drawText(0,
                    lineY - 2 * textPaddingPixels,
                    barBeatString);
        }
    }
}

QString WaveformRenderPlayMarker::generateBarBeatDisplayTextFromPrevBeat(mixxx::Beat beat) {
    return QString("%1%2%3").arg(
            QString::number(beat.getBarIndex() + 1),
            kBarBeatSeparator,
            QString::number(beat.getBarRelativeBeatIndex() + 1));
}