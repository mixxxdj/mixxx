#include "waveform/renderers/waveformrenderphrase.h"

#include <QPainter>

#include "track/phrases.h"
#include "track/track.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

class QPaintEvent;

WaveformRenderPhrase::WaveformRenderPhrase(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
}

void WaveformRenderPhrase::setup(const QDomNode& node, const SkinContext& context) {
    m_colors = mixxx::readPhraseColors(node, context);
}

void WaveformRenderPhrase::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer pTrackInfo = m_waveformRenderer->getTrackInfo();
    if (!pTrackInfo) {
        return;
    }

    mixxx::PhrasesPointer pPhrases = pTrackInfo->getPhrases();
    if (!pPhrases || pPhrases->isEmpty()) {
        return;
    }

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    PainterScope painterScope(painter);
    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const auto rendererWidth = static_cast<double>(m_waveformRenderer->getWidth());
    const auto rendererHeight = static_cast<double>(m_waveformRenderer->getHeight());

    for (const auto& phrase : pPhrases->phrases()) {
        const double startSample = phrase.startPosition().toEngineSamplePos();
        const double endSample = phrase.endPosition().toEngineSamplePos();

        const double startRatio = startSample / trackSamples;
        const double endRatio = endSample / trackSamples;
        if (startRatio > lastDisplayedPosition || endRatio < firstDisplayedPosition) {
            continue;
        }

        const double startX =
                m_waveformRenderer->transformSamplePositionInRendererWorld(startSample);
        const double endX =
                m_waveformRenderer->transformSamplePositionInRendererWorld(endSample);
        if (endX <= startX) {
            continue;
        }

        const QColor color = mixxx::phraseColor(m_colors, phrase.type());
        if (orientation == Qt::Horizontal) {
            painter->fillRect(
                    QRectF(QPointF(startX, 0.0), QPointF(endX, rendererHeight)), color);
        } else {
            painter->fillRect(
                    QRectF(QPointF(0.0, startX), QPointF(rendererWidth, endX)), color);
        }
    }
}
